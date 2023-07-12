import sys
from PyQt5.QtWidgets import QApplication, QWidget, QTabWidget, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QPushButton


class Tab1(QWidget):
    def __init__(self):
        super().__init__()
        layout = QVBoxLayout()
        label = QLabel("This is Tab 1")
        layout.addWidget(label)
        self.setLayout(layout)


class Tab2(QWidget):
    def __init__(self):
        super().__init__()
        layout = QVBoxLayout()
        label = QLabel("This is Tab 2")
        layout.addWidget(label)
        self.setLayout(layout)


class App(QWidget):
    def __init__(self):
        super().__init__()
        self.title = "Multi-Tab App"
        self.left = 100
        self.top = 100
        self.width = 400
        self.height = 300
        self.initUI()

    def initUI(self):
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)

        # Create tab bar and add tabs
        tabs = QTabWidget()
        tab1 = Tab1()
        tab2 = Tab2()
        tabs.addTab(tab1, "Tab 1")
        tabs.addTab(tab2, "Tab 2")

        # Add tabs to main layout
        layout = QVBoxLayout()
        layout.addWidget(tabs)

        self.setLayout(layout)

        self.show()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())
