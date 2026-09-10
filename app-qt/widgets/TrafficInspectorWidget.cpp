/// @file TrafficInspectorWidget.cpp
/// @brief Implementation of protocol inspector and injector widget.

#include "TrafficInspectorWidget.h"
#include "FujinonSX800Core/ProtocolParser.h"

#include <QDateTime>
#include <QHeaderView>
#include <QScrollBar>

namespace FujinonSX800App {

TrafficInspectorWidget::TrafficInspectorWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUi();
}

void TrafficInspectorWidget::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // Toolbar controls
    auto* topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    chkAutoScroll = new QCheckBox(tr("Auto Scroll"), this);
    chkAutoScroll->setChecked(true);

    cmbFilter = new QComboBox(this);
    cmbFilter->addItem(tr("All Traffic"), 0);
    cmbFilter->addItem(tr("TX (Commands) Only"), 1);
    cmbFilter->addItem(tr("RX (Replies) Only"), 2);

    btnClear = new QPushButton(tr("Clear Log"), this);

    topLayout->addWidget(chkAutoScroll);
    topLayout->addWidget(cmbFilter);
    topLayout->addWidget(btnClear);
    topLayout->addStretch();

    // Raw Hex Injector
    editRawHex = new QLineEdit(this);
    editRawHex->setPlaceholderText(tr("Hex bytes e.g. FF 07 00 20 00 00 27"));
    editRawHex->setFixedWidth(260);

    btnSendRaw = new QPushButton(tr("Send Hex"), this);
    btnSendRaw->setObjectName("btnPrimary");

    topLayout->addWidget(editRawHex);
    topLayout->addWidget(btnSendRaw);

    mainLayout->addLayout(topLayout);

    // Table view
    tableInspector = new QTableWidget(this);
    tableInspector->setColumnCount(4);
    tableInspector->setHorizontalHeaderLabels({ tr("Time"), tr("Direction"), tr("Hex Bytes"), tr("Decoded Command") });
    tableInspector->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableInspector->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tableInspector->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableInspector->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    tableInspector->verticalHeader()->setVisible(false);
    tableInspector->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableInspector->setAlternatingRowColors(true);
    tableInspector->setEditTriggers(QAbstractItemView::NoEditTriggers);

    mainLayout->addWidget(tableInspector);

    // Connections
    connect(btnClear, &QPushButton::clicked, this, &TrafficInspectorWidget::clearLog);
    connect(cmbFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        &TrafficInspectorWidget::handleFilterChanged);
    connect(btnSendRaw, &QPushButton::clicked, this, &TrafficInspectorWidget::handleSendClicked);
}

void TrafficInspectorWidget::logFrame(const QByteArray& frame, bool isTx)
{
    if (frame.isEmpty()) {
        return;
    }

    if (filterMode == 1 && !isTx) {
        return;
    }
    if (filterMode == 2 && isTx) {
        return;
    }

    const int row = tableInspector->rowCount();
    tableInspector->insertRow(row);

    const QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    const QString dirStr = isTx ? "TX  >>" : "RX  <<";

    std::vector<std::uint8_t> stdBytes(frame.size());
    for (qsizetype i { 0 }; i < frame.size(); ++i) {
        stdBytes[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(frame.at(i));
    }

    const QString hexStr = frame.toHex(' ').toUpper();
    const QString descStr = QString::fromStdString(FujinonSX800::ProtocolParser::describeFrame(stdBytes, isTx));

    auto* itemTime = new QTableWidgetItem(timeStr);
    auto* itemDir = new QTableWidgetItem(dirStr);
    auto* itemHex = new QTableWidgetItem(hexStr);
    auto* itemDesc = new QTableWidgetItem(descStr);

    const QColor colorTx { 88, 166, 255 }; // blue/cyan
    const QColor colorRx { 63, 185, 80 }; // green
    const QColor textColor = isTx ? colorTx : colorRx;

    itemDir->setForeground(textColor);
    itemHex->setForeground(textColor);
    itemDesc->setForeground(textColor);

    tableInspector->setItem(row, 0, itemTime);
    tableInspector->setItem(row, 1, itemDir);
    tableInspector->setItem(row, 2, itemHex);
    tableInspector->setItem(row, 3, itemDesc);

    // Limit log size to prevent memory bloat
    if (tableInspector->rowCount() > 500) {
        tableInspector->removeRow(0);
    }

    if (chkAutoScroll->isChecked()) {
        tableInspector->scrollToBottom();
    }
}

void TrafficInspectorWidget::clearLog()
{
    tableInspector->setRowCount(0);
}

void TrafficInspectorWidget::handleFilterChanged(int index)
{
    filterMode = index;
}

void TrafficInspectorWidget::handleSendClicked()
{
    const QString text = editRawHex->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    const QString sanitized = text.simplified().remove(' ');
    const QByteArray hexBytes = QByteArray::fromHex(sanitized.toUtf8());
    if (!hexBytes.isEmpty()) {
        emit sendRawHexRequested(hexBytes);
    }
}

} // namespace FujinonSX800App
