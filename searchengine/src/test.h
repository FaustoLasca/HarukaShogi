#ifndef TEST_H
#define TEST_H

class Test {
private:
    int value;
public:
    explicit Test(int value);
    int search(int depth);
};

#endif // TEST_H