#pragma once
#include <QRectF>
#include <QSizeF>
#include <QVector>

enum class ViewMode { SinglePage, Continuous, FacingPages, ContinuousFacing };

struct PagePlacement {
    int page = -1;
    QRectF rect;
    qreal pageToCanvasScale = 1.0;
};

struct LayoutResult {
    QVector<PagePlacement> pages;
    QSizeF canvasSize;
};

class LayoutManager {
public:
    static LayoutResult calculate(const QVector<QSizeF>& pageSizesPoints,
                                  ViewMode mode, int currentPage, qreal zoom,
                                  qreal viewportWidth, qreal margin = 28.0,
                                  qreal gap = 18.0);
private:
    static qreal centeredX(qreal viewportWidth, qreal width);
};
