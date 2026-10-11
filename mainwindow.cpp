#include "mainwindow.h"
#include <QFont>

// 未翻开格子的样式（灰色）
static const char *STYLE_UNREVEALED =
    "QPushButton { background-color:#D6D6D6; border:2px solid #AAA; border-radius:4px; }";

// 插旗格子的样式（灰色 + 红字）
static const char *STYLE_FLAGGED =
    "QPushButton { background-color:#D6D6D6; border:2px solid #AAA; border-radius:4px; color:#C0392B; font-weight:bold; }";

// 数字 1~8 的经典扫雷配色
static QString numberStyle(int n)
{
    static const char *colors[] = {"", "#0000FF", "#008000", "#FF0000",
                                   "#000080", "#800000", "#008080", "#000000", "#808080"};
    return QString("QPushButton { background-color:#F5F5F5; border:1px solid #CCC; color:%1; font-weight:bold; }")
        .arg(colors[n]);
}

// ===== CellButton 实现 =====

CellButton::CellButton(int r, int c, QWidget *parent)
    : QPushButton(parent), row(r), col(c)
{
    setMinimumSize(40, 40); // 格子尺寸
    setFont(QFont("Arial", 14, QFont::Bold));
    setStyleSheet(STYLE_UNREVEALED);
}

void CellButton::mouseReleaseEvent(QMouseEvent *e)
{
    QPushButton::mouseReleaseEvent(e); // 先交给父类处理按压状态
    // 左键发 leftClicked，右键发 rightClicked，坐标随信号带出
    if (e->button() == Qt::LeftButton)
    {
        emit leftClicked(row, col);
    }
    else if (e->button() == Qt::RightButton)
    {
        emit rightClicked(row, col);
    }
}

// ===== MainWindow 实现 =====

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("扫雷 Minesweeper");
    setFixedSize(430, 520);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ---- 顶部栏：剩余雷数 + 状态提示 + 新游戏按钮 ----
    QHBoxLayout *topLayout = new QHBoxLayout();

    label_mines = new QLabel("剩余雷数: 10", this);
    label_mines->setFont(QFont("Arial", 12, QFont::Bold));

    label_status = new QLabel("左键翻开，右键插旗", this);

    QPushButton *btn_restart = new QPushButton("新游戏", this);
    btn_restart->setStyleSheet(
        "QPushButton { background-color:#3498DB; color:white; border:none; border-radius:6px; padding:6px 14px; }"
        "QPushButton:hover { background-color:#2980B9; }");
    connect(btn_restart, &QPushButton::clicked, this, &MainWindow::onRestartClicked);

    topLayout->addWidget(label_mines);
    topLayout->addWidget(label_status, 1); // 1 = 占据中间剩余空间
    topLayout->addWidget(btn_restart);
    mainLayout->addLayout(topLayout);

    // ---- 棋盘 ----
    buildBoard();
    mainLayout->addLayout(grid_board);
    newGame();
}

// 创建 9x9 按钮网格，连接信号槽
void MainWindow::buildBoard()
{
    grid_board = new QGridLayout();
    grid_board->setSpacing(2);

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            CellButton *btn = new CellButton(r, c, this);
            cells[r][c] = btn;
            grid_board->addWidget(btn, r, c);

            // 左键 -> 翻开，右键 -> 插旗
            connect(btn, &CellButton::leftClicked, this, &MainWindow::onCellRevealed);
            connect(btn, &CellButton::rightClicked, this, &MainWindow::onCellFlagged);
        }
    }
}

// 左键槽函数：翻开格子
void MainWindow::onCellRevealed(int r, int c)
{
    if (game.isGameOver())
    {
        return;
    }


    bool alive = game.reveal(r, c);

    if (!alive)
    {
        game.revealAllMines(); // 踩雷：翻开所有雷展示给玩家
        label_status->setText("游戏结束：踩雷了！");
    }
    else if (game.isWin())
    {

        label_status->setText("恭喜通关！");
    }

    refreshBoard();
}

// 右键槽函数：插旗 / 取消插旗
void MainWindow::onCellFlagged(int r, int c)
{
    game.toggleFlag(r, c);
    refreshBoard();
}

// 新游戏按钮
void MainWindow::onRestartClicked()
{
    newGame();
}

// 开始新一局
void MainWindow::newGame()
{
    if (!game.init(ROWS, COLS, MINES))
    {
        label_status->setText("棋盘参数非法！");
        return;
    }
    label_status->setText("左键翻开，右键插旗");
    refreshBoard();
}

// 按核心逻辑状态刷新全部按钮的显示
void MainWindow::refreshBoard()
{
    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            CellButton *btn = cells[r][c];

            if (game.isRevealed(r, c))
            {
                if (game.isMine(r, c))
                {
                    // 翻开的雷：红底 + 雷符号
                    btn->setText("*");
                    btn->setStyleSheet(
                        "QPushButton { background-color:#E74C3C; border:1px solid #999; color:white; font-weight:bold; }");
                }
                else if (game.adjacentMines(r, c) == 0)
                {
                    // 空白格
                    btn->setText("");
                    btn->setStyleSheet("QPushButton { background-color:#F5F5F5; border:1px solid #CCC; }");
                }
                else
                {
                    // 数字格：显示数字并按经典配色上色
                    btn->setText(QString::number(game.adjacentMines(r, c)));
                    btn->setStyleSheet(numberStyle(game.adjacentMines(r, c)));
                }
            }
            else if (game.isFlagged(r, c))
            {
                // 插旗
                btn->setText("F");
                btn->setStyleSheet(STYLE_FLAGGED);
            }
            else
            {
                // 未翻开
                btn->setText("");
                btn->setStyleSheet(STYLE_UNREVEALED);
            }
        }
    }

    // 更新剩余雷数
    label_mines->setText("剩余雷数: " + QString::number(game.remainingMines()));
}