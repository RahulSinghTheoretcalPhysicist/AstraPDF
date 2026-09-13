#pragma once
#include <QString>
struct CosSummary {
    bool validPdf=false;
    QString version;
    qint64 fileSize=0;
    qint64 startXref=-1;
    QString trailerSnippet;
    QString eofStatus;
    QString humanReadable;
};
class CosInspector { public: static CosSummary inspect(const QString& filePath); };
