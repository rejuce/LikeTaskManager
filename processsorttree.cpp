#include "processsorttree.h"
#include <QHeaderView>
#include <QHash>
#include <algorithm>
#include <functional>

ProcessSortTree::ProcessSortTree(QWidget* parent)
    : QTreeWidget(parent)
{
    // Never use built-in sorting — it's lexicographic.
    // We handle all sorting manually with numeric comparison.
    setSortingEnabled(false);
    setRootIsDecorated(true);
    setUniformRowHeights(true);
    header()->setSortIndicatorShown(true);
    header()->setSortIndicator(0, Qt::AscendingOrder);

    // Handle header clicks manually (sectionPressed fires even when
    // setSortingEnabled is false, unlike sectionClicked)
    connect(header(), &QHeaderView::sectionPressed,
            this, &ProcessSortTree::onSectionClicked);
}

void ProcessSortTree::onSectionClicked(int logicalIndex)
{
    // Toggle direction if clicking the same column, else ascending
    if (logicalIndex == m_sortColumn) {
        m_sortOrder = (m_sortOrder == Qt::AscendingOrder)
                          ? Qt::DescendingOrder
                          : Qt::AscendingOrder;
    } else {
        m_sortColumn = logicalIndex;
        m_sortOrder = Qt::AscendingOrder;
    }

    header()->setSortIndicator(m_sortColumn, m_sortOrder);
    sortByColumn(m_sortColumn, m_sortOrder);
}

void ProcessSortTree::applyNumericSort()
{
    sortByColumn(m_sortColumn, m_sortOrder);
}

void ProcessSortTree::sortByColumn(int column, Qt::SortOrder order)
{
    // Collect top-level items (preserving identity & expansion state)
    QVector<QTreeWidgetItem*> items;
    items.reserve(topLevelItemCount());
    for (int i = 0; i < topLevelItemCount(); ++i)
        items.append(topLevelItem(i));

    if (items.size() <= 1)
        return;

    // Numeric comparator: try to parse UserRole as double, fall back to string
    auto cmp = [column](const QTreeWidgetItem* a, const QTreeWidgetItem* b) -> bool {
        QVariant va = a->data(column, Qt::UserRole);
        QVariant vb = b->data(column, Qt::UserRole);
        if (va.isValid() && vb.isValid()) {
            bool okA = false, okB = false;
            double numA = va.toDouble(&okA);
            double numB = vb.toDouble(&okB);
            if (okA && okB)
                return numA < numB;
        }
        return a->text(column).toLower() < b->text(column).toLower();
    };

    if (order == Qt::AscendingOrder) {
        std::stable_sort(items.begin(), items.end(), cmp);
    } else {
        std::stable_sort(items.begin(), items.end(),
                         [cmp](const QTreeWidgetItem* a, const QTreeWidgetItem* b) {
                             return cmp(b, a);
                         });
    }

    // Reorder in-place.  Sorting is disabled, so addTopLevelItem won't
    // trigger any built-in re-sort.
    //
    // QTreeWidget::takeTopLevelItem() collapses the taken item AND all
    // its descendants (Qt behaviour), so we must recursively save every
    // item's expansion state before removing and restore it afterwards —
    // otherwise every update tick would collapse all groups and PIDs,
    // making the tree unreadable.
    QHash<QTreeWidgetItem*, bool> wasExpanded;
    std::function<void(QTreeWidgetItem*)> saveExpansion =
        [&](QTreeWidgetItem* item) {
            wasExpanded.insert(item, item->isExpanded());
            for (int i = 0; i < item->childCount(); ++i)
                saveExpansion(item->child(i));
        };
    for (auto* item : items)
        saveExpansion(item);

    setUpdatesEnabled(false);
    while (topLevelItemCount() > 0)
        takeTopLevelItem(0);
    for (auto* item : items)
        addTopLevelItem(item);

    std::function<void(QTreeWidgetItem*)> restoreExpansion =
        [&](QTreeWidgetItem* item) {
            item->setExpanded(wasExpanded.value(item, false));
            for (int i = 0; i < item->childCount(); ++i)
                restoreExpansion(item->child(i));
        };
    for (auto* item : items)
        restoreExpansion(item);

    setUpdatesEnabled(true);
}
