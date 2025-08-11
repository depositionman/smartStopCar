#ifndef __BASETASK_H
#define __BASETASK_H

#include <iostream>

class BaseTask
{
public:
    BaseTask();
    ~BaseTask();
    virtual void working() = 0;
protected:
private:
};

#endif