#include "LayoutManager.h"
#include <algorithm>

qreal LayoutManager::centeredX(qreal viewportWidth, qreal width)
{
    return std::max<qreal>(28.0, (viewportWidth - width) / 2.0);
}

LayoutResult LayoutManager::calculate(const QVector<QSizeF>& pageSizes,
                                      ViewMode mode, int currentPage, qreal zoom,
                                      qreal viewportWidth, qreal margin, qreal gap)
{
    LayoutResult out;
    if (pageSizes.isEmpty()) return out;

    viewportWidth = std::max<qreal>(viewportWidth, 320.0);
    zoom = std::clamp<qreal>(zoom, 0.20, 5.00);

    auto addSingle = [&](int page, qreal& y) {
        const QSizeF pts = pageSizes.at(page);
        const qreal w = pts.width() * zoom;
        const qreal h = pts.height() * zoom;
        const qreal x = centeredX(viewportWidth, w);
        out.pages.push_back({page, QRectF(x, y, w, h), zoom});
        y += h + gap;
        out.canvasSize.setWidth(std::max(out.canvasSize.width(), x + w + margin));
        out.canvasSize.setHeight(y + margin);
    };

    if (mode == ViewMode::SinglePage) {
        currentPage = std::clamp(currentPage, 0, pageSizes.size() - 1);
        qreal y = margin; addSingle(currentPage, y);
        out.canvasSize.setWidth(std::max(viewportWidth, out.canvasSize.width()));
        return out;
    }

    if (mode == ViewMode::Continuous) {
        qreal y = margin;
        for (int p=0; p<pageSizes.size(); ++p) addSingle(p, y);
        out.canvasSize.setWidth(std::max(viewportWidth, out.canvasSize.width()));
        return out;
    }

    auto addPair = [&](int leftPage, int rightPage, qreal& y) {
        const QSizeF leftPts = pageSizes.at(leftPage);
        const bool hasRight = rightPage >= 0 && rightPage < pageSizes.size();
        const QSizeF rightPts = hasRight ? pageSizes.at(rightPage) : QSizeF();

        const qreal commonHeight =
            std::max(leftPts.height(), hasRight ? rightPts.height() : leftPts.height()) * zoom;
        const qreal leftScale = commonHeight / leftPts.height();
        const qreal leftW = leftPts.width() * leftScale;

        qreal rightScale = zoom, rightW = 0.0;
        if (hasRight) {
            rightScale = commonHeight / rightPts.height();
            rightW = rightPts.width() * rightScale;
        }

        const qreal totalW = leftW + (hasRight ? gap + rightW : 0.0);
        const qreal x0 = centeredX(viewportWidth, totalW);

        out.pages.push_back({leftPage, QRectF(x0, y, leftW, commonHeight), leftScale});
        if (hasRight)
            out.pages.push_back({rightPage, QRectF(x0+leftW+gap, y, rightW, commonHeight), rightScale});

        y += commonHeight + gap;
        out.canvasSize.setWidth(std::max(out.canvasSize.width(), x0 + totalW + margin));
        out.canvasSize.setHeight(y + margin);
    };

    if (mode == ViewMode::FacingPages) {
        currentPage = std::clamp(currentPage, 0, pageSizes.size() - 1);
        int left = currentPage;
        if (left % 2 == 1) --left;
        qreal y = margin; addPair(left, left+1, y);
        out.canvasSize.setWidth(std::max(viewportWidth, out.canvasSize.width()));
        return out;
    }

    qreal y = margin;
    for (int left=0; left<pageSizes.size(); left+=2) addPair(left, left+1, y);
    out.canvasSize.setWidth(std::max(viewportWidth, out.canvasSize.width()));
    return out;
}
