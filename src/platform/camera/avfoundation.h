/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */
#pragma once

#include "src/video/videomode.h"

#include <QPair>
#include <QString>
#include <QVector>

#ifdef Q_OS_MACOS
namespace avfoundation {
bool isDesktopCapture(const QString& devName);
bool hasPermission(const QString& devName);
QVector<VideoMode> getDeviceModes(const QString& devName);
QVector<QPair<QString, QString>> getDeviceList();
} // namespace avfoundation
#endif
