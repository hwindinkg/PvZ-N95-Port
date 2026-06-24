#ifndef INSETS_H
#define INSETS_H

struct Insets
{
    int mLeft;
    int mTop;
    int mRight;
    int mBottom;

    Insets() : mLeft(0), mTop(0), mRight(0), mBottom(0) {}
    Insets(int l, int t, int r, int b)
        : mLeft(l), mTop(t), mRight(r), mBottom(b) {}
};

#endif // INSETS_H
