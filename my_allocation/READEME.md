简介: 该项目实现了 STL 标准库中空间配置器(allocate)

空间配置器分两级：一级空间配置器与二级空间配置器。

该空间配置器设计非常巧妙，尤其是在 chunk_alloc 这个函数当中



设计流程如下: 

一级空间配置器

a.根据对象的大小，直接 malloc / realloc 申请空间

  如果成功: 返回 result

  如果失败: 调用 _oom_malloc(size_t) / _oom_realloc(void*, size_t);

b.在 _oom_malloc(size_t) / _oom_realloc(void*, size_t) 中，判断 _malloc_alloc_oom_handler 是否为 nullptr

  如果为 nullptr: 异常处理，throw std::bad_alloc();

  如果不为 nullptr: 调用 _malloc_alloc_oom_handler() 释放空间

  继续 malloc, 判断 malloc 返回值是否为 nullptr

      (1)如果为 nullptr, 继续循环调用 _malloc_alloc_oom_handler();

      (2)如果不为 nullptr, 那么返回 result;


二级空间配置器 

a.先判断申请内存是否大于 128K 

  如果大于: 那么直接调用一级空间配置器(MallocAllocTemplate::_allocate(size_t bytes));

  否则调用二级空间配置器(DefaultAllocTemplate::_allocate(size_t bytes));

b.根据申请的空间，计算出在自由链表中对应的内存块(DefaultAllocTemplate::_get_free_list_index(size_t bytes));

c.到对应的内存块处判断该处是否有空闲内存块(DefaultAllocTemplate::_refill(size_t n));

  如果有: 那么直接取出并返回

  否则去内存池里进行申请(DefaultAllocTemplate::_chunk_alloc(size_t n, size_t nobjs)); nobjs 一般来说是 20

d.判断内存池是否有超过 nobjs 个对象大小的内存

  如果内存池内存 >= nobjs * 对象大小: 直接分配 nobjs 个，返回一个，剩下 nobjs - 1 个挂在自由链表对应位置

  如果内存池内存 > 一个对象所需要的内存并且 < nobjs * 对象大小: 那么分配内存池内存 / 对象大小个内存块出去，返回一个，剩下的挂在自由链表

  如果内存池内存小于一个对象的内存，先将内存池的剩余内存挂在对应自由链表的位置，接着去向 heap 申请

      (1)如果在堆上 malloc �功，那么改变自由链表的 _start_free, _end_free, _heap_size 接着再次调用； _chunk_alloc 

      (2)如果在堆上申请失败的话，那么在自由链表上查找是否有大于对象的空间

         如果自由链表上有大�对象的空间: 那么返回一块空间给内存池，再次调用 _chunk_alloc

         如果自由链表上没有大于对象的空间: 那么调用一级空间适配器
