#include "src/widget/loginscreen.h"

#include <QDebug>
#include <QLineEdit>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QTest>

#include "src/persistence/paths.h"
#include "src/widget/style.h"

namespace {
std::pair<QPixmap, QPixmap> compareScreenshot(QWidget* widget, const QString& path)
{
    QPixmap actual = widget->grab();
    const auto env = QProcessEnvironment::systemEnvironment();
    if (env.contains("BUILD_WORKSPACE_DIRECTORY")) {
        const QString actualPath = QStringLiteral("%1/qtox/test/resources/images/%2")
                                       .arg(env.value("BUILD_WORKSPACE_DIRECTORY"), path);
        qDebug() << "Saving actual image to" << actualPath;
        actual.save(actualPath);
    }
    QPixmap expected = QPixmap(QStringLiteral(":/test/images/%1").arg(path));
    return {std::move(actual), std::move(expected)};
}

#define COMPARE_GRAB(widget, path)                                                 \
    do {                                                                           \
        const auto [actual, expected] = compareScreenshot(widget, path);           \
        if (!QTest::qCompare(actual, expected, #widget, path, __FILE__, __LINE__)) \
            QTEST_FAIL_ACTION;                                                     \
    } while (false)

} // namespace

class TestLoginScreen : public QObject
{
    Q_OBJECT

private slots:
    void testLoginScreen();
    void testCreateProfile();
    void testCreateProfileBadPassword();

private:
    Paths paths{Paths::Portable::Portable};
    Style style;
    int themeColor = 0; // Default
    QString profileName = "test-user";
};

void TestLoginScreen::testLoginScreen()
{
    LoginScreen loginScreen(paths, style, themeColor, profileName);

    COMPARE_GRAB(&loginScreen, "loginscreen_empty.png");
}

void TestLoginScreen::testCreateProfile()
{
    LoginScreen loginScreen(paths, style, themeColor, profileName);

    bool created = false;
    QObject::connect(&loginScreen, &LoginScreen::createNewProfile, this,
                     [&created]() { created = true; });

    loginScreen.findChild<QLineEdit*>("newUsername")->setText("test-user");
    loginScreen.findChild<QLineEdit*>("newPass")->setText("password");
    loginScreen.findChild<QLineEdit*>("newPassConfirm")->setText("password");
    loginScreen.findChild<QPushButton*>("createAccountButton")->click();

    QVERIFY(created);
    COMPARE_GRAB(&loginScreen, "loginscreen_ok.png");
}

void TestLoginScreen::testCreateProfileBadPassword()
{
    LoginScreen loginScreen(paths, style, themeColor, profileName);

    bool created = false;
    connect(&loginScreen, &LoginScreen::createNewProfile, this, [&created]() { created = true; });

    QString error;
    connect(&loginScreen, &LoginScreen::failure, this,
            [&error](const QString& title, const QString& message) { error = message; });

    loginScreen.findChild<QLineEdit*>("newUsername")->setText("test-user");
    loginScreen.findChild<QLineEdit*>("newPass")->setText("password");
    loginScreen.findChild<QLineEdit*>("newPassConfirm")->setText("password2");
    QTest::mouseClick(loginScreen.findChild<QPushButton*>("createAccountButton"),
                      Qt::MouseButton::LeftButton);

    QVERIFY(!created);
    QVERIFY(error.contains("different"));
}

QTEST_MAIN(TestLoginScreen)
#include "loginscreen_test.moc"
