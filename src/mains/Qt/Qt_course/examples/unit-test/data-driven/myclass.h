#ifndef MYCLASS_H
#define MYCLASS_H

class MyClass
{
public:
    MyClass() : _x(0), _y(1) {}
    MyClass( int x, int y ) : _x(x), _y(y) {}
    int devide()
    {
        return _x / _y;
    }

private:
    int _x;
    int _y;
};

Q_DECLARE_METATYPE( MyClass )


#endif /* MYCLASS_H */

