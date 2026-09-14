#include "PdfCanvas.h"
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPdfDocument>
#include <QPdfDocumentRenderOptions>
#include <algorithm>
#include <cmath>

PdfCanvas::PdfCanvas(QWidget *parent):QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void PdfCanvas::setDocument(QPdfDocument *document)
{
    m_document=document;
    m_currentPage=0;
    m_fitToWidth=true;
    m_zoom=1.0;
    m_selection.reset();
    m_searchSelection.reset();
    m_highlights.clear();
    clearRenderCache();
    rebuildLayout();
}

void PdfCanvas::setViewportWidth(qreal width)
{
    m_viewportWidth=std::max<qreal>(width,320.0);
    rebuildLayout();
}

void PdfCanvas::setViewMode(ViewMode mode)
{
    if(m_mode==mode)return;
    m_mode=mode;
    rebuildLayout();
}

void PdfCanvas::setCurrentPage(int page)
{
    if(!m_document||m_document->pageCount()<=0)return;
    page=std::clamp(page,0,m_document->pageCount()-1);
    m_currentPage=page;
    if(m_mode==ViewMode::SinglePage||m_mode==ViewMode::FacingPages||m_fitToWidth) rebuildLayout();
    if(const auto *p=placementForPage(page)) emit requestEnsureVisible(p->rect);
    emit currentPageChanged(page);
}

qreal PdfCanvas::effectiveZoom() const
{
    if(!m_fitToWidth || !m_document || m_document->pageCount()<=0)
        return m_zoom;

    QSizeF page=m_document->pagePointSize(std::clamp(m_currentPage,0,m_document->pageCount()-1));
    if(page.width()<=0.0 || page.height()<=0.0)
        page=QSizeF(595.2756,841.8898);

    const qreal outerGutter=18.0;
    const qreal usable=std::max<qreal>(260.0,m_viewportWidth-2.0*outerGutter);
    return std::clamp<qreal>(usable/page.width(),0.20,5.00);
}

qreal PdfCanvas::zoom() const
{
    return effectiveZoom();
}

void PdfCanvas::setZoom(qreal zoom)
{
    zoom=std::clamp<qreal>(zoom,0.20,5.00);
    m_fitToWidth=false;
    if(qFuzzyCompare(m_zoom,zoom)){
        rebuildLayout();
        return;
    }
    m_zoom=zoom;
    clearRenderCache();
    rebuildLayout();
}

void PdfCanvas::fitToWidth()
{
    m_fitToWidth=true;
    clearRenderCache();
    rebuildLayout();
    emit statusMessage("Fit Width: page fills the reading area while preserving the PDF aspect ratio.");
}

void PdfCanvas::setNeonBackground(const QColor& color)
{
    if(!color.isValid()) return;
    m_neonColor=color;
    int hue=color.hsvHue();
    if(hue<0) hue=190;
    m_backgroundColor=QColor::fromHsv(hue,150,24);
    update();
}

void PdfCanvas::zoomIn(){ setZoom(effectiveZoom()*1.15); }
void PdfCanvas::zoomOut(){ setZoom(effectiveZoom()/1.15); }

QVector<QSizeF> PdfCanvas::pageSizes() const
{
    QVector<QSizeF> s;
    if(!m_document)return s;

    const int count=m_document->pageCount();
    if(count<=0)return s;

    const QSizeF fallback(595.2756,841.8898);
    s.fill(fallback,count);

    if(m_mode==ViewMode::SinglePage){
        const QSizeF actual=m_document->pagePointSize(m_currentPage);
        if(actual.isValid() && actual.width()>0 && actual.height()>0) s[m_currentPage]=actual;
        return s;
    }

    if(m_mode==ViewMode::FacingPages){
        int left=m_currentPage;
        if(left%2==1)--left;
        const QSizeF leftActual=m_document->pagePointSize(left);
        if(leftActual.isValid() && leftActual.width()>0 && leftActual.height()>0) s[left]=leftActual;
        if(left+1<count){
            const QSizeF rightActual=m_document->pagePointSize(left+1);
            if(rightActual.isValid() && rightActual.width()>0 && rightActual.height()>0) s[left+1]=rightActual;
        }
        return s;
    }

    for(int i=0;i<count;++i){
        const QSizeF actual=m_document->pagePointSize(i);
        if(actual.isValid() && actual.width()>0 && actual.height()>0) s[i]=actual;
    }
    return s;
}

void PdfCanvas::rebuildLayout()
{
    if(!m_document||m_document->pageCount()<=0){
        m_layout={};
        resize(qMax(320,int(m_viewportWidth)),600);
        update();
        return;
    }

    const qreal layoutZoom=effectiveZoom();
    m_layout=LayoutManager::calculate(pageSizes(),m_mode,m_currentPage,layoutZoom,m_viewportWidth,18.0,18.0);
    resize(qMax(320,int(std::ceil(m_layout.canvasSize.width()))),
           qMax(600,int(std::ceil(m_layout.canvasSize.height()))));
    update();
}

QString PdfCanvas::cacheKey(int page,QSize size) const
{ return QStringLiteral("%1:%2x%3").arg(page).arg(size.width()).arg(size.height()); }

QImage PdfCanvas::renderedPage(int page,QSize pixelSize)
{
    if(!m_document||pixelSize.isEmpty())return {};
    const QString key=cacheKey(page,pixelSize);
    auto it=m_cache.constFind(key);
    if(it!=m_cache.constEnd())return it.value();

    const QImage raw=m_document->render(page,pixelSize,QPdfDocumentRenderOptions{});
    if(raw.isNull())return {};

    QImage image(pixelSize,QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    {
        QPainter pagePainter(&image);
        pagePainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        pagePainter.drawImage(QPoint(0,0),raw);
    }

    while(m_cache.size()>=16)m_cache.erase(m_cache.begin());
    m_cache.insert(key,image);
    return image;
}
void PdfCanvas::clearRenderCache(){ m_cache.clear(); }

const PagePlacement* PdfCanvas::placementForPage(int page) const
{
    for(const auto& p:m_layout.pages)if(p.page==page)return &p;
    return nullptr;
}

QPointF PdfCanvas::canvasToPage(const QPointF& c,const PagePlacement& p) const
{ return {(c.x()-p.rect.left())/p.pageToCanvasScale,(c.y()-p.rect.top())/p.pageToCanvasScale}; }

QPointF PdfCanvas::pageToCanvas(const QPointF& q,const PagePlacement& p) const
{ return {p.rect.left()+q.x()*p.pageToCanvasScale,p.rect.top()+q.y()*p.pageToCanvasScale}; }

QPolygonF PdfCanvas::pagePolygonToCanvas(const QPolygonF& poly,const PagePlacement& p) const
{
    QPolygonF out; out.reserve(poly.size());
    for(const auto& pt:poly)out<<pageToCanvas(pt,p);
    return out;
}

PdfCanvas::Hit PdfCanvas::hitTest(const QPointF& pt) const
{
    for(const auto& p:m_layout.pages)
        if(p.rect.contains(pt))return {true,p.page,canvasToPage(pt,p),p};
    return {};
}

void PdfCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(),m_backgroundColor);
    const QRectF dirty=event->rect();

    for(const auto& p:m_layout.pages){
        if(!p.rect.adjusted(-12,-12,12,12).intersects(dirty))continue;

        QColor glow=m_neonColor;
        glow.setAlpha(22);
        painter.fillRect(p.rect.adjusted(-12,-12,12,12),glow);
        glow.setAlpha(38);
        painter.fillRect(p.rect.adjusted(-7,-7,7,7),glow);
        glow.setAlpha(70);
        painter.fillRect(p.rect.adjusted(-3,-3,3,3),glow);
        painter.fillRect(p.rect,Qt::white);

        QSize px(qMax(1,int(std::round(p.rect.width()))),qMax(1,int(std::round(p.rect.height()))));
        QImage image=renderedPage(p.page,px);
        if(!image.isNull())painter.drawImage(p.rect,image);
        QColor border=m_neonColor;
        border.setAlpha(155);
        painter.setPen(border);
        painter.drawRect(p.rect);

        for(const auto& ann:m_highlights)if(ann.page==p.page){
            painter.save(); painter.setPen(Qt::NoPen); painter.setBrush(QColor(255,235,59,95));
            for(const auto& poly:ann.polygons)painter.drawPolygon(pagePolygonToCanvas(poly,p));
            painter.restore();
        }
        if(m_searchSelection && m_searchSelection->isValid() && p.page==m_searchPage){
            painter.save(); painter.setPen(Qt::NoPen); painter.setBrush(QColor(255,152,0,110));
            for(const auto& poly:m_searchSelection->bounds())painter.drawPolygon(pagePolygonToCanvas(poly,p));
            painter.restore();
        }
        if(m_selection && m_selection->isValid() && p.page==m_selectionPage){
            painter.save(); painter.setPen(Qt::NoPen); painter.setBrush(QColor(50,130,255,90));
            for(const auto& poly:m_selection->bounds())painter.drawPolygon(pagePolygonToCanvas(poly,p));
            painter.restore();
        }
    }
}

void PdfCanvas::mousePressEvent(QMouseEvent *event)
{
    if(event->button()!=Qt::LeftButton||!m_document)return QWidget::mousePressEvent(event);
    Hit hit=hitTest(event->position()); if(!hit.valid)return;
    m_dragging=true; m_selectionPage=hit.page; m_selectionStart=hit.pagePoint; m_selectionEnd=hit.pagePoint;
    m_selection.reset(); setCurrentPage(hit.page); update();
}

void PdfCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_dragging||!m_document)return QWidget::mouseMoveEvent(event);
    Hit hit=hitTest(event->position()); if(!hit.valid||hit.page!=m_selectionPage)return;
    m_selectionEnd=hit.pagePoint;
    m_selection = m_document->getSelection(m_selectionPage,m_selectionStart,m_selectionEnd);
    emit selectionChanged((m_selection && m_selection->isValid()) ? m_selection->text() : QString()); update();
}

void PdfCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)m_dragging=false;
    QWidget::mouseReleaseEvent(event);
}

bool PdfCanvas::copySelection()
{
    if(!m_selection || !m_selection->isValid())return false;
    QApplication::clipboard()->setText(m_selection->text());
    emit statusMessage("Selected text copied."); return true;
}

bool PdfCanvas::addHighlightFromSelection()
{
    if(!m_selection || !m_selection->isValid())return false;
    HighlightAnnotation a; a.page=m_selectionPage; a.polygons=m_selection->bounds();
    m_highlights.push_back(a); update();
    emit statusMessage("Highlight added to overlay layer."); return true;
}

bool PdfCanvas::findNext(const QString& raw)
{
    if(!m_document||raw.trimmed().isEmpty())return false;
    const QString text=raw.trimmed();
    int startPage=0,startIndex=0;
    if(m_lastSearch.compare(text,Qt::CaseInsensitive)==0&&m_searchPage>=0){
        startPage=m_searchPage; startIndex=m_searchIndex+text.size();
    }else{m_searchPage=-1;m_searchIndex=-1;}

    const int count=m_document->pageCount();
    for(int pass=0;pass<2;++pass){
        const int first=pass==0?startPage:0;
        const int last=pass==0?count:startPage+1;
        for(int page=first;page<last;++page){
            const QString pageText=m_document->getAllText(page).text();
            const int from=(pass==0&&page==startPage)?startIndex:0;
            const int idx=pageText.indexOf(text,from,Qt::CaseInsensitive);
            if(idx>=0){
                m_lastSearch=text;m_searchPage=page;m_searchIndex=idx;
                m_searchSelection = m_document->getSelectionAtIndex(page,idx,text.size());
                setCurrentPage(page);
                if(const auto *placement=placementForPage(page)){
                    QRectF target=placement->rect;
                    if(m_searchSelection && m_searchSelection->isValid()){
                        QRectF b=m_searchSelection->boundingRectangle();
                        target=QRectF(pageToCanvas(b.topLeft(),*placement),
                                      pageToCanvas(b.bottomRight(),*placement)).normalized().adjusted(-40,-40,40,40);
                    }
                    emit requestEnsureVisible(target);
                }
                update(); emit statusMessage(QStringLiteral("Found on page %1.").arg(page+1)); return true;
            }
        }
    }
    emit statusMessage("Text not found."); return false;
}
