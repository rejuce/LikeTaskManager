#ifndef PROCESSSORTTREE_H
#define PROCESSSORTTREE_H

#include <QTreeWidget>

/**
 * QTreeWidget subclass that performs numeric sorting on columns that carry
 * a double in Qt::UserRole.  The default QTreeWidget sort is lexicographic
 * (string), which gives wrong order for values like "9.0" vs "26.0".
 *
 * Built-in sorting is always disabled.  Header clicks are handled via the
 * header's sectionClicked signal, and all reordering is done manually.
 */
class ProcessSortTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ProcessSortTree(QWidget* parent = nullptr);

    /// Re-sort top-level items using numeric (UserRole) comparison.
    /// Call this after updating item values in handle_process_data().
    void applyNumericSort();

    /// Sort top-level items by the given column and order (preserves expansion).
    void sortByColumn(int column, Qt::SortOrder order);

    int sortColumn() const { return m_sortColumn; }
    Qt::SortOrder sortOrder() const { return m_sortOrder; }

private:
    void onSectionClicked(int logicalIndex);

    int m_sortColumn = 0;
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};

#endif // PROCESSSORTTREE_H
