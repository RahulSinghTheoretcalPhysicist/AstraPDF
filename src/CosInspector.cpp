#include "CosInspector.h"
#include <QFile>
#include <QRegularExpression>

CosSummary CosInspector::inspect(const QString& filePath)
{
    CosSummary s;
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly)) { s.humanReadable="Could not open file."; return s; }

    s.fileSize=f.size();
    const QByteArray head=f.read(32);
    QRegularExpression verRe(QStringLiteral(R"(%PDF-(\d\.\d))"));
    auto vm=verRe.match(QString::fromLatin1(head));
    if (vm.hasMatch()) { s.validPdf=true; s.version=vm.captured(1); }

    const qint64 tailSize=qMin<qint64>(8192,s.fileSize);
    f.seek(s.fileSize-tailSize);
    const QByteArray tail=f.read(tailSize);
    const QString tailText=QString::fromLatin1(tail);

    QRegularExpression startRe(QStringLiteral(R"(startxref\s+(\d+))"));
    auto it=startRe.globalMatch(tailText);
    QString last;
    while (it.hasNext()) last=it.next().captured(1);
    if (!last.isEmpty()) s.startXref=last.toLongLong();

    s.eofStatus=tail.contains("%%EOF") ? "%%EOF found" : "%%EOF not found in final 8 KiB";
    const int pos=tail.lastIndexOf("trailer");
    s.trailerSnippet = pos>=0 ? QString::fromLatin1(tail.mid(pos,900))
                              : QStringLiteral("No classic trailer token found; file may use an xref stream.");

    s.humanReadable=QStringLiteral(
        "PDF structural summary\n\nValid header: %1\nVersion: %2\nFile size: %3 bytes\n"
        "startxref byte offset: %4\nEOF: %5\n\nTrailer / tail snippet:\n%6")
        .arg(s.validPdf?"yes":"no")
        .arg(s.version.isEmpty()?"unknown":s.version)
        .arg(s.fileSize).arg(s.startXref).arg(s.eofStatus).arg(s.trailerSnippet);
    return s;
}
