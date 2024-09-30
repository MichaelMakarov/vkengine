#pragma once

class allocator_interface {
  public:
    virtual ~allocator_interface() = default;

    virtual void *allocate() = 0;

    virtual void deallocate(void *) = 0;
};