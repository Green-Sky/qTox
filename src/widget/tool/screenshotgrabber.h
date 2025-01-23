/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/widget/tool/abstractscreenshotgrabber.h"

#include <QObject>
#include <QPixmap>
#include <QPointer>

class QGraphicsSceneMouseEvent;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class QGraphicsScene;
class QGraphicsView;
class QKeyEvent;
class ScreenGrabberChooserRectItem;
class ScreenGrabberOverlayItem;
class ToolBoxGraphicsItem;

class ScreenshotGrabber : public AbstractScreenshotGrabber
{
    Q_OBJECT
public:
    explicit ScreenshotGrabber(QObject* parent);
    ~ScreenshotGrabber() override;

    bool eventFilter(QObject* object, QEvent* event) override;

    void showGrabber() override;

private slots:
    void acceptRegion();
    void reInit();

private:
    friend class ScreenGrabberOverlayItem;
    bool mKeysBlocked;

    void setupScene();

    void useNothingSelectedTooltip();
    void useRegionSelectedTooltip();
    void chooseHelperTooltipText(QRect rect);
    void adjustTooltipPosition();

    bool handleKeyPress(QKeyEvent* event);
    void reject();

    QPixmap grabScreen() const;

    void hideVisibleWindows();
    void restoreHiddenWindows();

    void beginRectChooser(QGraphicsSceneMouseEvent* event);

private:
    QPixmap screenGrab;
    QGraphicsScene* scene;
    QGraphicsView* window;
    QGraphicsPixmapItem* screenGrabDisplay;
    ScreenGrabberOverlayItem* overlay;
    ScreenGrabberChooserRectItem* chooserRect;
    ToolBoxGraphicsItem* helperToolbox;
    QGraphicsTextItem* helperTooltip;

    qreal pixRatio = 1.0;

    bool mQToxVisible;
    QVector<QPointer<QWidget>> mHiddenWindows;
};
