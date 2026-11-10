#ifndef HOME_PAGE_H
#define HOME_PAGE_H

#include <QPixmap>
#include <QVector>
#include <QWidget>

class QLabel;
class QPushButton;
class CellFlashAnimator;

class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(QWidget* parent = nullptr);

    void startIntroAnimation();
    void stopIntroAnimation();

    int decorativeCellCount() const { return cells_.size(); }
    CellFlashAnimator* animator() const { return animator_; }
    QPushButton* startButton() const { return startButton_; }
    QLabel* titleLabel() const { return titleLabel_; }
    QWidget* rightGridContainer() const { return rightGridContainer_; }
    bool hasBackgroundPixmap() const { return !bgPixmap_.isNull(); }

signals:
    void startRequested();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap bgPixmap_;
    QVector<QWidget*> cells_;
    CellFlashAnimator* animator_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QPushButton* startButton_ = nullptr;
    QWidget* rightGridContainer_ = nullptr;
};

#endif // HOME_PAGE_H
