#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>

class QShapeCanvas;

class shape {
public:
    virtual void paint(QPainter &painter) const = 0;

    [[nodiscard]] virtual bool contains(int x, int y) const = 0;

    void select() {
        isSelected = true;
    }

    void move(int dx, int dy) {
        m_x += dx;
        m_y += dy;
    }

    void unselect() {
        isSelected = false;
    }

protected:
    int m_x, m_y;
    bool isSelected;

    explicit shape(int x, int y) : m_x{x}, m_y{y}, isSelected{false} {}
};

class rectangle : public shape {
public:
    explicit rectangle(int x, int y, int width, int height) : shape(x, y),
                                                              m_width{width}, m_height{height} {}

    void paint(QPainter &painter) const override {
        painter.setBrush(QColor(255, 0, 255));
        isSelected ? painter.setPen(QColor(255, 0, 0)) : painter.setPen(Qt::NoPen);
        painter.drawRect(m_x, m_y, m_width, m_height);
    }

    [[nodiscard]] bool contains(int x, int y) const override {
        return x >= m_x && x < (m_x + m_width) &&
               y >= m_y && y < (m_y + m_height);
    }

private:
    int m_width, m_height;
};

class circle : public shape {
public:
    explicit circle(int x, int y, int radius) : shape(x, y), m_radius{radius} {}

    void paint(QPainter &painter) const override {
        painter.setBrush(QColor(255, 255, 0));
        isSelected ? painter.setPen(QColor(0, 0, 255)) : painter.setPen(Qt::NoPen);
        painter.drawEllipse(m_x, m_y, m_radius * 2, m_radius * 2);
    }

    [[nodiscard]] bool contains(int x, int y) const override {
        int dx = x - m_x - m_radius;
        int dy = y - m_y - m_radius;
        return (dx * dx + dy * dy) < (m_radius * m_radius);
    }

private:
    int m_radius;
};

class QShapeCanvas : public QWidget {
public:

    enum ShapesToDraw {
        CircleShape,
        RectangleShape,
        None
    };

    explicit QShapeCanvas(QWidget *parent = nullptr) : QWidget(parent) {
        setMinimumSize(200, 200);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setFocusPolicy(Qt::StrongFocus);
        nextShape = None;
        selectedShape = nullptr;
    }

    void addShape(const std::shared_ptr<shape> &shape) {
        shapes.push_back(shape);
    }

    void setNextShape(ShapesToDraw shape) {
        nextShape = shape;
    }


protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter;
        painter.begin(this);
        for (auto &shape: shapes)
            shape->paint(painter);
        painter.end();
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (selectedShape != nullptr) {
            selectedShape->unselect();
            selectedShape = nullptr;
        }
        if (event->button() == Qt::LeftButton) {
            if (nextShape == RectangleShape) {
                addShape(std::make_shared<rectangle>(event->pos().x() - 25, event->pos().y() - 25, 50, 50));
            } else if (nextShape == CircleShape) {
                addShape(std::make_shared<circle>(event->pos().x() - 50, event->pos().y() - 50, 50));
            }
            update();
        } else if (event->button() == Qt::RightButton) {
            for (auto it = shapes.rbegin(); it != shapes.rend(); it++) {
                if ((*it)->contains(event->pos().x(), event->pos().y())) {
                    selectedShape = *it;
                    (*it)->select();
                    prev_x = event->pos().x();
                    prev_y = event->pos().y();
                    break;
                }
            }
            update();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (selectedShape) {
            if (nextShape == QShapeCanvas::ShapesToDraw::RectangleShape) {
                int dx = event->pos().x() - prev_x;
                int dy = event->pos().y() - prev_y;
                selectedShape->move(dx, dy);
                prev_x = event->pos().x();
                prev_y = event->pos().y();
            } else if (nextShape == QShapeCanvas::ShapesToDraw::CircleShape) {
                int dx = event->pos().x() - prev_x;
                int dy = event->pos().y() - prev_y;
                selectedShape->move(dx, dy);
                prev_x = event->pos().x();
                prev_y = event->pos().y();
//                selectedShape->move(event->pos().x() + 50, event->pos().y() + 50);
            }
            update();
        }
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (selectedShape && (event->key() == Qt::Key::Key_Backspace || event->key() == Qt::Key::Key_Delete)) {
            selectedShape->unselect();
            for (auto it = shapes.begin(); it != shapes.end(); it++) {
                if ((*it) == selectedShape) {
                    shapes.erase(it);
                    selectedShape = nullptr;
                    break;
                }
            }
        }
        update();
    }

private:
    ShapesToDraw nextShape;
    std::vector<std::shared_ptr<shape>> shapes;
    std::shared_ptr<shape> selectedShape;
    int prev_x, prev_y;
};

class QPaintWindow : public QWidget {
public:
    QPaintWindow() {
        this->setWindowTitle("Problem #5");
        this->resize(500, 500);

        auto *canvas = new QShapeCanvas(this);
        canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        auto *controlPanel = new QWidget(this);

        auto *circleButton = new QPushButton("Circle", controlPanel);
        auto *rectangleButton = new QPushButton("Rectangle", controlPanel);
        circleButton->setStyleSheet("background-color: white;" "color: black;");
        rectangleButton->setStyleSheet("background-color: white;" "color: black;");

        QObject::connect(rectangleButton, &QPushButton::clicked, [canvas, rectangleButton, circleButton]() {
            rectangleButton->setStyleSheet("background-color: red;" "color: white;");
            circleButton->setStyleSheet("background-color: white;" "color: black;");
            canvas->setNextShape(QShapeCanvas::ShapesToDraw::RectangleShape);
        });

        QObject::connect(circleButton, &QPushButton::clicked, [canvas, rectangleButton, circleButton]() {
            rectangleButton->setStyleSheet("background-color: white;" "color: black;");
            circleButton->setStyleSheet("background-color: red;" "color: white;");
            canvas->setNextShape(QShapeCanvas::ShapesToDraw::CircleShape);
        });

        auto *buttonLayout = new QHBoxLayout(controlPanel);
        buttonLayout->addWidget(circleButton);
        buttonLayout->addWidget(rectangleButton);

        controlPanel->setLayout(buttonLayout);

        auto *windowLayout = new QVBoxLayout(this);
        windowLayout->addWidget(canvas);
        windowLayout->addWidget(controlPanel);

        this->setLayout(windowLayout);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    auto *window = new QPaintWindow();

    window->show();

    return QApplication::exec();
}
