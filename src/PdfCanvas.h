#pragma once
#include <QColor>
#include <QImage>
#include <QMap>
#include <QPolygonF>
#include <optional>
#include <QPdfSelection>
#include <QVector>
#include <QWidget>
#include "LayoutManager.h"

class QPdfDocument;

struct HighlightAnnotation {
    int page=-1;
    QList<QPolygonF> polygons;
};

class PdfCanvas final : public QWidget {
    Q_OBJECT
public:
    explicit PdfCanvas(QWidget *parent=nullptr);
    void setDocument(QPdfDocument *document);
    void setViewportWidth(qreal width);
    void setViewMode(ViewMode mode);
    ViewMode viewMode() const { return m_mode; }
    void setCurrentPage(int page);
    int currentPage() const { return m_currentPage; }
    void setZoom(qreal zoom);
    qreal zoom() const;
    void fitToWidth();
    bool isFitToWidth() const { return m_fitToWidth; }
    void zoomIn();
    void zoomOut();
    bool findNext(const QString& text);
    bool copySelection();
    bool addHighlightFromSelection();
    void clearRenderCache();
    void setNeonBackground(const QColor& color);
    QColor neonBackgroundColor() const { return m_neonColor; }

signals:
    void currentPageChanged(int page);
    void requestEnsureVisible(const QRectF& canvasRect);
    void selectionChanged(const QString& text);
    void statusMessage(const QString& message);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    struct Hit { bool valid=false; int page=-1; QPointF pagePoint; PagePlacement placement; };
    void rebuildLayout();
    QVector<QSizeF> pageSizes() const;
    qreal effectiveZoom() const;
    Hit hitTest(const QPointF& canvasPoint) const;
    QPointF canvasToPage(const QPointF&, const PagePlacement&) const;
    QPointF pageToCanvas(const QPointF&, const PagePlacement&) const;
    QPolygonF pagePolygonToCanvas(const QPolygonF&, const PagePlacement&) const;
    const PagePlacement* placementForPage(int page) const;
    QString cacheKey(int page, QSize pixelSize) const;
    QImage renderedPage(int page, QSize pixelSize);

    QPdfDocument *m_document=nullptr;
    ViewMode m_mode=ViewMode::SinglePage;
    int m_currentPage=0;
    qreal m_zoom=1.0;
    qreal m_viewportWidth=1000.0;
    bool m_fitToWidth=true;
    LayoutResult m_layout;
    QMap<QString,QImage> m_cache;
    QColor m_neonColor=QColor("#00e5ff");
    QColor m_backgroundColor=QColor(5,18,22);

    bool m_dragging=false;
    int m_selectionPage=-1;
    QPointF m_selectionStart, m_selectionEnd;
    std::optional<QPdfSelection> m_selection;

    int m_searchPage=-1, m_searchIndex=-1;
    QString m_lastSearch;
    std::optional<QPdfSelection> m_searchSelection;
    QVector<HighlightAnnotation> m_highlights;
};
