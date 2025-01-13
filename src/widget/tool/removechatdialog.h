/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once


#include "ui_removechatdialog.h"
#include "src/model/friend.h"
#include <QDialog>


class RemoveChatDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RemoveChatDialog(QWidget* parent, const Chat& contact);

    bool removeHistory() const
    {
        return ui.removeHistory->isChecked();
    }

    bool accepted() const
    {
        return _accepted;
    }

public slots:
    void onAccepted();

protected:
    Ui_RemoveChatDialog ui;
    bool _accepted = false;
};
