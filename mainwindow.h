#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include "MineSweeper.h"

// 棋盘规格：9x9、10 雷（初级难度）
const int ROWS = 9;
const int COLS = 9;
const int MINES = 10;

// 自定义格子按钮：区分鼠标左键（翻开）与右键（插旗）
class CellButton : public QPushButton
{
    Q_OBJECT
public:
    CellButton(int r, int c, QWidget *parent = nullptr);

    int row; // 格子行号
    int col; // 格子列号

    signals:
        void leftClicked(int row, int col);  // 左键 = 翻开
    void rightClicked(int row, int col); // 右键 = 插旗

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;
};

// 主窗口
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onCellRevealed(int row, int col); // 左键槽：翻开
    void onCellFlagged(int row, int col);  // 右键槽：插旗
    void onRestartClicked();               // 新游戏槽

private:
    void buildBoard();   // 创建 9x9 按钮网格并连接信号槽
    void refreshBoard(); // 按核心逻辑状态刷新全部按钮显示
    void newGame();      // 开始新一局

    MineSweeper game;              // 核心逻辑对象（与界面分离）
    CellButton *cells[ROWS][COLS]; // 按钮二维数组
    QGridLayout *grid_board;       // 棋盘网格布局
    QLabel *label_mines;           // 剩余雷数标签
    QLabel *label_status;          // 状态提示标签
};

#endif // MAINWINDOW_H