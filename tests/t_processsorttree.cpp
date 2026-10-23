// Unit tests for ProcessSortTree — numeric sort + expansion preservation.
#include <QtTest>
#include <QTreeWidgetItem>
#include "processsorttree.h"

class TestProcessSortTree : public QObject
{
    Q_OBJECT

private:
    static QTreeWidgetItem* make_group(ProcessSortTree* tree,
                                       const QString& name,
                                       double cpuValue,
                                       int childCount = 0)
    {
        auto* item = new QTreeWidgetItem();
        item->setText(0, name);
        item->setData(0, Qt::UserRole, cpuValue);
        tree->addTopLevelItem(item);
        for (int i = 0; i < childCount; ++i) {
            auto* child = new QTreeWidgetItem();
            child->setText(0, QString("pid_%1").arg(i));
            child->setData(0, Qt::UserRole, i + 1);
            item->addChild(child);
        }
        return item;
    }

private slots:
    void numeric_sort_ascending();
    void numeric_sort_descending();
    void expansion_preserved_after_sort();
    void child_expansion_preserved_after_sort();
    void no_reorder_when_single_item();
};

void TestProcessSortTree::numeric_sort_ascending()
{
    ProcessSortTree tree;
    make_group(&tree, "zebra",  30.0);
    make_group(&tree, "alpha", 10.0);
    make_group(&tree, "mid",   20.0);

    tree.applyNumericSort();

    QCOMPARE(tree.topLevelItemCount(), 3);
    QCOMPARE(tree.topLevelItem(0)->text(0), QString("alpha"));
    QCOMPARE(tree.topLevelItem(1)->text(0), QString("mid"));
    QCOMPARE(tree.topLevelItem(2)->text(0), QString("zebra"));
}

void TestProcessSortTree::numeric_sort_descending()
{
    ProcessSortTree tree;
    make_group(&tree, "alpha", 10.0);
    make_group(&tree, "zebra",  30.0);
    make_group(&tree, "mid",   20.0);

    // Sort descending by column 0
    tree.sortByColumn(0, Qt::DescendingOrder);

    QCOMPARE(tree.topLevelItemCount(), 3);
    QCOMPARE(tree.topLevelItem(0)->text(0), QString("zebra"));
    QCOMPARE(tree.topLevelItem(1)->text(0), QString("mid"));
    QCOMPARE(tree.topLevelItem(2)->text(0), QString("alpha"));
}

void TestProcessSortTree::expansion_preserved_after_sort()
{
    ProcessSortTree tree;
    auto* a = make_group(&tree, "alpha", 10.0, 2);
    auto* b = make_group(&tree, "beta",  20.0, 3);
    auto* c = make_group(&tree, "gamma", 30.0, 1);

    // Expand some, leave others collapsed
    a->setExpanded(true);
    b->setExpanded(false);
    c->setExpanded(true);

    tree.applyNumericSort();

    // Items should be in order alpha, beta, gamma (ascending)
    QCOMPARE(tree.topLevelItem(0)->text(0), QString("alpha"));
    QCOMPARE(tree.topLevelItem(1)->text(0), QString("beta"));
    QCOMPARE(tree.topLevelItem(2)->text(0), QString("gamma"));

    // Expansion state must be preserved
    QCOMPARE(tree.topLevelItem(0)->isExpanded(), true);   // alpha was expanded
    QCOMPARE(tree.topLevelItem(1)->isExpanded(), false);  // beta was collapsed
    QCOMPARE(tree.topLevelItem(2)->isExpanded(), true);   // gamma was expanded
}

void TestProcessSortTree::child_expansion_preserved_after_sort()
{
    ProcessSortTree tree;
    auto* a = make_group(&tree, "alpha", 10.0, 3);
    auto* b = make_group(&tree, "beta",  20.0, 3);

    // Expand parent "alpha" and one of its children
    a->setExpanded(true);
    a->child(1)->setExpanded(true);
    b->setExpanded(true);

    // Force a reorder: sort descending
    tree.sortByColumn(0, Qt::DescendingOrder);

    // beta is now first, alpha second
    QCOMPARE(tree.topLevelItem(0)->text(0), QString("beta"));
    QCOMPARE(tree.topLevelItem(1)->text(0), QString("alpha"));

    // Both parents still expanded
    QCOMPARE(tree.topLevelItem(0)->isExpanded(), true);
    QCOMPARE(tree.topLevelItem(1)->isExpanded(), true);

    // Child expansion preserved (alpha's child[1] was expanded)
    QCOMPARE(tree.topLevelItem(1)->child(1)->isExpanded(), true);
}

void TestProcessSortTree::no_reorder_when_single_item()
{
    ProcessSortTree tree;
    auto* a = make_group(&tree, "solo", 5.0, 1);
    a->setExpanded(true);

    tree.applyNumericSort();

    QCOMPARE(tree.topLevelItemCount(), 1);
    QCOMPARE(tree.topLevelItem(0)->text(0), QString("solo"));
    QCOMPARE(tree.topLevelItem(0)->isExpanded(), true);
}

QTEST_MAIN(TestProcessSortTree)
#include "t_processsorttree.moc"
