#pragma once

#include <iostream>
#include <cstdlib>

typedef void(*MALLOCALLOC)();                             /* 定义 void (*)() 函数指针类型 */

template <int inst>
class MallocAllocTemplate {                               /* 一级空间配置器 */
private:
  static void* _oom_malloc(size_t);                       /* malloc 失败所调用的函数 */
  static void* _oom_realloc(void* , size_t);              /* realloc 失败所调用的函数 */
  static MALLOCALLOC _malloc_alloc_oom_handler;           /* 释放内存的函数 */
public:
  static void* _allocate(size_t);                         /* malloc 申请内存 */
  static void* _reallocate(void* , size_t);               /* realloc 申请内存 */
  static void _deallocate(void*);                         /* free 释放内存 */
  static MALLOCALLOC _set_malloc_handler(MALLOCALLOC);    /* 设置释放内存的函数 */
};

template <int inst>
void (* MallocAllocTemplate<inst>::_malloc_alloc_oom_handler)() = nullptr;

template <int inst>
void* MallocAllocTemplate<inst>::_oom_malloc(size_t n) {
  if (n <= 0)
    return nullptr;

  void* result = nullptr;
  while (1) {
    if (_malloc_alloc_oom_handler == nullptr)
      throw std::bad_alloc();

    (*_malloc_alloc_oom_handler)();

    result = (void*)malloc(n);
    if (result != nullptr)
      break;
  }
  return result;
}

template <int inst>
void* MallocAllocTemplate<inst>::_oom_realloc(void* ptr, size_t n) {
  if (n <= 0)
    return nullptr;

  void* result = nullptr;
  while (1) {
    if (_malloc_alloc_oom_handler == nullptr)
      throw std::bad_alloc();

    (*_malloc_alloc_oom_handler)();

    result = (void*)realloc(ptr, n);
    if (result != nullptr)
      break;
  }
  return result;
}

template <int inst>
void* MallocAllocTemplate<inst>::_allocate(size_t n) {
  if (n <= 0)
    return nullptr;

  void* result = (void*)malloc(n);
  if (result == nullptr)
    result = _oom_malloc(n);
  return result;
}

template <int inst>
void* MallocAllocTemplate<inst>::_reallocate(void* ptr, size_t n) {
  if (n <= 0)
    return nullptr;

  void* result = nullptr;
  result = (void*)realloc(ptr, n);
  if (result == nullptr)
    result = _oom_realloc(ptr, n);
  return result;
}

template <int inst>
void MallocAllocTemplate<inst>::_deallocate(void* ptr) {
  if (ptr == nullptr)
    return;
  free(ptr);
}

template <int inst>
MALLOCALLOC MallocAllocTemplate<inst>::_set_malloc_handler(MALLOCALLOC fun) {
  MALLOCALLOC old_fun = _malloc_alloc_oom_handler;
  _malloc_alloc_oom_handler = fun;
  return old_fun;
}

typedef MallocAllocTemplate<0> _malloc_alloc;
enum { _ALIGN = 8 };                                   /* 按照基准值为 8 字节操作 */
enum { _MAXBYTES = 128  };                             /* 最大上限为 128 字节 */
enum { _NFREELISTS = _ALIGN / _MAXBYTES  };            /* 自由链表的节点数 */
enum { _NOBJS = 20  };                                 /* 每次向内存池申请的对象个数 */

template <bool threads, int inst>
class DefaultAllocTemplate {
  union _obj {                                         /* 自由链表内节点 */
    _obj* _free_list_link;                             /* 保存下一个节点的地址 */
  };
private:
  static char* _start_free;                            /* 内存池开始位置 */
  static char* _end_free;                              /* 内存池��束位置 */
  static size_t _heap_size;                            /* 已经向堆申请的空间大小 */
  static _obj* volatile _free_list[_NFREELISTS];       /* 自由链表 */
private:
  static char* _chunk_alloc(size_t, size_t&);          /* 向内存池申请对象 */
  static void* _refill(size_t);                        /* 向自由链表申请内存 */
  static size_t _get_free_list_index(size_t bytes);    /* 根据申请内存的大小计算出自由链表对应的下标 */
  static size_t _get_round_up(size_t bytes);           /* 根据申请内存的大小提升至 8 的倍数 */
public:
  static void* _allocate(size_t);                      /* 申请内存 */
  static void _deallocate(void*, size_t);              /* 释放内存 */

};

template <bool threads, int inst>
char* DefaultAllocTemplate<threads, inst>::_end_free = nullptr;

template <bool threads, int inst>
char* DefaultAllocTemplate<threads, inst>::_start_free = nullptr;

template <bool threads, int inst>
size_t DefaultAllocTemplate<threads, inst>::_heap_size = 0;

template <bool threads, int inst>
typename DefaultAllocTemplate<threads, inst>::_obj* volatile
DefaultAllocTemplate<threads, inst>::_free_list[_NFREELISTS] = { NULL  };

template <bool threads, int inst>
size_t DefaultAllocTemplate<threads, inst>::_get_free_list_index(size_t bytes) {
  return (bytes + (size_t)_ALIGN - 1) / (size_t)_ALIGN - 1;
}

template <bool threads, int inst>
size_t DefaultAllocTemplate<threads, inst>::_get_round_up(size_t bytes) {
  return _get_free_list_index(bytes) * _ALIGN + _ALIGN;

}

template <bool threads, int inst>
void* DefaultAllocTemplate<threads, inst>::_allocate(size_t bytes) {
  if (bytes >= _MAXBYTES)
    return _malloc_alloc::_allocate(bytes);

  void* ret = nullptr;
  _obj** volatile my_free_list = _free_list + _get_free_list_index(bytes);
  if (*my_free_list != nullptr) {
    ret = *my_free_list;
    *my_free_list = (*my_free_list)->_free_list_link;
  }
  else {
    ret = _refill(_get_round_up(bytes));
  }
  return ret;
}

template <bool threads, int inst>
void DefaultAllocTemplate<threads, inst>::_deallocate(void* ptr, size_t n) {
  if (n >= _MAXBYTES) 
    return _malloc_alloc::_deallocate(ptr);

  _obj* q = (_obj*)ptr;
  _obj** volatile tmp = _free_list + _get_free_list_index(n);
  q->_free_list_link = *tmp;
  *tmp = q;
}

template <bool threads, int inst>
char* DefaultAllocTemplate<threads, inst>::_chunk_alloc(size_t n, size_t& nobjs) {
  char* result = nullptr;
  size_t totalBytes = n * nobjs;                              /* 计算需要的内存 */
  size_t leftBytes = _end_free - _start_free;                 /* 查看内存池剩余的内存 */

  if (leftBytes >= totalBytes) {                              /* 如果内存池内存大于所需要的 */
    result = _start_free;
    _start_free += totalBytes;
    return result;
  }
  else if (leftBytes >= n) {                                  /* 内存池剩余内存还能够分配至少一个内存块 */
    nobjs = (size_t)leftBytes / n;
    result = _start_free;
    _start_free += (n * nobjs);
    return result;
  }
  else {   
    if (leftBytes > 0) {                                      /* 如果内存池还有剩余*/
      _obj** my_free_list = _free_list + _get_free_list_index(leftBytes);
      ((_obj*)_start_free)->_free_list_link = *my_free_list;
      *my_free_list = (_obj*)_start_free;
    }

    size_t newBytes = 2 * totalBytes + (_heap_size >> 4);     /* 需要的大小 * 2 加上_heap_size / 16 */
    _start_free = (char*)malloc(newBytes);

    if (_start_free == nullptr) {                             /* 如果 _start_free 不为 nullptr，意味着申请成功 */
      for (size_t i = n; i <= _MAXBYTES; i += _ALIGN) {       /* 如果 _start_free 为 nullptr, 申请失败，那么此时在自由链表中查找，是否有满足的内存，归还 */
        _obj** volatile my_free_list = _free_list + _get_free_list_index(i);
        _obj* cur = *my_free_list;
        if (cur != nullptr) {
          *my_free_list = cur->_free_list_link;
          _start_free = (char*)cur;
          _end_free = _start_free + i;
          return _chunk_alloc(n, nobjs);                      /* 如果找到了，就归还，继续调用 _chunk_alloc */
        }
      }
      _end_free = nullptr;
      _start_free = (char*)_malloc_alloc::_allocate(newBytes);      
    }
    _heap_size += newBytes;                                    /* 如果开辟成功，那么更新 _heap_size，更新 _end_free*/
    _end_free = _start_free + newBytes;
    return _chunk_alloc(n, nobjs);
  }
}

template <bool threads, int inst>
void* DefaultAllocTemplate<threads, inst>::_refill(size_t n) {
  size_t nobjs = _NOBJS;
  char* chunk = _chunk_alloc(n, nobjs);

  if (nobjs == 1)
    return chunk;

  _obj* ret = (_obj*)chunk;
  _obj** volatile my_free_list = _free_list + _get_free_list_index(n);
  *my_free_list = (_obj*)chunk + 1;
  _obj* cur = *my_free_list;
  cur->_free_list_link = nullptr;
  _obj* next = nullptr;
  for (size_t i = 2; i < nobjs; ++i) {
    next = (_obj*)chunk + i;
    cur->_free_list_link = next;
    cur = next;
  }
  cur->_free_list_link = nullptr;

  return ret;
}
