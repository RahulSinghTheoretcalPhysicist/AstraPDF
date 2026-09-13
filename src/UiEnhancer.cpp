#include "UiEnhancer.h"
#include "MainWindow.h"

#include <QComboBox>
#include <QDesktopServices>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextBrowser>
#include <QTextDocumentFragment>
#include <QUrl>
#include <QUrlQuery>
#include <QWidget>

UiEnhancer::UiEnhancer(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if(!m_window) return;
    m_network=m_window->findChild<QNetworkAccessManager*>();
    for(QDockWidget *dock:m_window->findChildren<QDockWidget*>()) decorateDock(dock);
    setupSearch();
}

void UiEnhancer::decorateDock(QDockWidget *dock)
{
    if(!dock || dock->property("astraDecorated").toBool()) return;
    dock->setProperty("astraDecorated",true);
    dock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);

    auto *bar=new QWidget(dock);
    auto *row=new QHBoxLayout(bar);
    row->setContentsMargins(8,3,4,3);
    row->setSpacing(4);
    auto *title=new QLabel(dock->windowTitle(),bar);
    title->setStyleSheet("font-weight:600;");
    auto *minButton=new QPushButton("_",bar);
    auto *floatButton=new QPushButton("↗",bar);
    auto *closeButton=new QPushButton("×",bar);
    for(QPushButton *b:{minButton,floatButton,closeButton}){
        b->setFixedSize(28,24);
        b->setFlat(true);
    }
    minButton->setToolTip("Minimize / hide panel");
    floatButton->setToolTip("Float or dock panel");
    closeButton->setToolTip("Close panel");
    row->addWidget(title,1);
    row->addWidget(minButton);
    row->addWidget(floatButton);
    row->addWidget(closeButton);
    dock->setTitleBarWidget(bar);

    connect(minButton,&QPushButton::clicked,dock,[dock]{
        if(dock->isFloating()) dock->showMinimized();
        else dock->hide();
    });
    connect(floatButton,&QPushButton::clicked,dock,[dock]{
        dock->setFloating(!dock->isFloating());
        dock->show();
        dock->raise();
    });
    connect(closeButton,&QPushButton::clicked,dock,&QWidget::hide);
}

void UiEnhancer::setupSearch()
{
    m_webDock=m_window->findChild<QDockWidget*>("webSearchDock");
    m_webInput=m_window->findChild<QLineEdit*>("webSearchInput");
    m_provider=m_window->findChild<QComboBox*>("webSearchProvider");
    m_webResult=m_window->findChild<QTextBrowser*>("webSearchResult");
    if(!m_webDock || !m_webInput || !m_provider || !m_webResult || !m_network) return;

    const auto buttons=m_webDock->findChildren<QPushButton*>();
    for(QPushButton *button:buttons){
        if(button->text().compare("Search",Qt::CaseInsensitive)==0){ m_webButton=button; break; }
    }
    if(!m_webButton) return;

    QObject::disconnect(m_webButton,nullptr,nullptr,nullptr);
    QObject::disconnect(m_webInput,&QLineEdit::returnPressed,nullptr,nullptr);
    connect(m_webButton,&QPushButton::clicked,this,&UiEnhancer::runSearch);
    connect(m_webInput,&QLineEdit::returnPressed,this,&UiEnhancer::runSearch);

    m_webResult->setOpenLinks(false);
    m_webResult->setOpenExternalLinks(false);
    connect(m_webResult,&QTextBrowser::anchorClicked,this,[](const QUrl& url){ QDesktopServices::openUrl(url); });
}

void UiEnhancer::runSearch()
{
    if(!m_webInput || !m_provider || !m_webResult) return;
    const QString query=m_webInput->text().trimmed();
    if(query.isEmpty()){
        m_webResult->setHtml("<h3>Web Search</h3><p>Enter something to search.</p>");
        return;
    }
    m_activeQuery=query;
    m_webResult->setHtml(QStringLiteral("<h2>%1</h2><p><i>Searching…</i></p>").arg(query.toHtmlEscaped()));
    const int mode=m_provider->currentIndex();
    if(mode==0 || mode==1) runDuckDuckGo(query);
    if(mode==0 || mode==2) runWikipedia(query);
}

void UiEnhancer::runDuckDuckGo(const QString& query)
{
    QUrl url("https://html.duckduckgo.com/html/");
    QUrlQuery params;
    params.addQueryItem("q",query);
    url.setQuery(params);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,"Mozilla/5.0 AstraPDF/0.2");
    request.setTransferTimeout(10000);
    QNetworkReply *reply=m_network->get(request);
    connect(reply,&QNetworkReply::finished,this,[this,reply,query]{
        const QByteArray data=reply->readAll();
        const bool ok=reply->error()==QNetworkReply::NoError;
        reply->deleteLater();
        if(query!=m_activeQuery || !m_webResult) return;

        QString html="<hr><h3>DuckDuckGo</h3>";
        const QUrl fallback(QStringLiteral("https://duckduckgo.com/?q=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(query))));
        if(!ok){
            html += QStringLiteral("<p>Could not load embedded results.</p><p><a href=\"%1\">Open full DuckDuckGo search</a></p>").arg(fallback.toString().toHtmlEscaped());
            m_webResult->append(html);
            return;
        }

        const QString page=QString::fromUtf8(data);
        QRegularExpression re(QStringLiteral("<a[^>]*class=\"result__a\"[^>]*href=\"([^\"]+)\"[^>]*>(.*?)</a>"),QRegularExpression::DotMatchesEverythingOption|QRegularExpression::CaseInsensitiveOption);
        auto it=re.globalMatch(page);
        int shown=0;
        while(it.hasNext() && shown<10){
            const auto m=it.next();
            QString link=m.captured(1);
            const QString title=QTextDocumentFragment::fromHtml(m.captured(2)).toPlainText().trimmed();
            if(link.startsWith("//")) link.prepend("https:");
            if(title.isEmpty() || link.isEmpty()) continue;
            html += QStringLiteral("<p><b><a href=\"%1\">%2</a></b></p>").arg(link.toHtmlEscaped(),title.toHtmlEscaped());
            ++shown;
        }
        if(shown==0) html += "<p>No embedded result was parsed.</p>";
        html += QStringLiteral("<p><a href=\"%1\">Open full DuckDuckGo search</a></p>").arg(fallback.toString().toHtmlEscaped());
        m_webResult->append(html);
    });
}

void UiEnhancer::runWikipedia(const QString& query)
{
    QUrl url("https://en.wikipedia.org/w/api.php");
    QUrlQuery params;
    params.addQueryItem("action","query");
    params.addQueryItem("list","search");
    params.addQueryItem("srsearch",query);
    params.addQueryItem("format","json");
    params.addQueryItem("utf8","1");
    params.addQueryItem("srlimit","8");
    url.setQuery(params);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,"AstraPDF/0.2");
    request.setTransferTimeout(10000);
    QNetworkReply *reply=m_network->get(request);
    connect(reply,&QNetworkReply::finished,this,[this,reply,query]{
        const QByteArray data=reply->readAll();
        const bool ok=reply->error()==QNetworkReply::NoError;
        reply->deleteLater();
        if(query!=m_activeQuery || !m_webResult) return;

        QString html="<hr><h3>Wikipedia</h3>";
        if(!ok){
            html += "<p>Wikipedia is unavailable right now.</p>";
            m_webResult->append(html);
            return;
        }
        const QJsonDocument doc=QJsonDocument::fromJson(data);
        const QJsonArray results=doc.object().value("query").toObject().value("search").toArray();
        if(results.isEmpty()){
            html += "<p>No Wikipedia results found.</p>";
            m_webResult->append(html);
            return;
        }
        for(const QJsonValue& value:results){
            const QJsonObject item=value.toObject();
            const QString title=item.value("title").toString();
            QString cleanTitle=title;
            cleanTitle.replace(' ','_');
            const QString snippet=QTextDocumentFragment::fromHtml(item.value("snippet").toString()).toPlainText();
            const QUrl article(QStringLiteral("https://en.wikipedia.org/wiki/%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(cleanTitle))));
            html += QStringLiteral("<p><b><a href=\"%1\">%2</a></b><br>%3</p>").arg(article.toString().toHtmlEscaped(),title.toHtmlEscaped(),snippet.toHtmlEscaped());
        }
        m_webResult->append(html);
    });
}
