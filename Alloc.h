#pragma once
typedef void(*HNADLE_FUNC)();//函数指针
template <int inst>//预留参数
//一级空间配置器
class __MallocAllocTemplate
{
	static HNADLE_FUNC __malloc_alloc_oom_handle;

	static HNADLE_FUNC SetMallocHandler(HNADLE_FUNC f)
	{
		HNADLE_FUNC old = __malloc_alloc_oom_handle;
		__malloc_alloc_oom_handle = f;
		return old;
	}
	static void* oom_malloc(size_t n)
	{
		while (1)
		{
			if (__malloc_alloc_oom_handle == 0)
			{
				throw bad_alloc();

			}
			else
			{
				__malloc_alloc_oom_handle();
				void* ret = malloc(n);
				if (ret)
				{
					return ret;
				}
			}
		}
	}
public:
	static void* Allocate(size_t n)
	{
		void *result = malloc(n);
		if (0 == result) 
			result = oom_malloc(n);
		return result;
	}

	static void Deallocate(void *p, size_t /* n */)
	{
		free(p);
	}
};

template<int inst>
HNADLE_FUNC __MallocAllocTemplate<inst>::__malloc_alloc_oom_handle = 0;

void FreeMemory()
{
	cout << "释放内存" << endl;
}

void test()
{
	__MallocAllocTemplate<0>::Allocate(1000);
}

// 二级空间配置器
template <bool threads, int inst>
class __DefaultAllocTemplate
{
private:
	enum { __ALIGN = 8 };
	enum { __MAX_BYTES = 128 };
	enum { __NFREELISTS = __MAX_BYTES / __ALIGN };
	union Obj
	{
		union Obj * _freeListLink;
		char client_data[1];    /* The client sees this.        */
	};
	//内存池
	static char *_startFree;
	static char *_endFree;
	static size_t _heapsize;
	//自由链表
	static Obj*  _freeList[__NFREELISTS];

	

public:
	static  size_t FREELIST_INDEX(size_t n)
	{
		return (((n)+__ALIGN - 1) / __ALIGN - 1);
	}

	static size_t ROUND_UP(size_t n)
	{
		return (((n)+__ALIGN - 1) & ~(__ALIGN - 1));
	}

	static char* ChunkAlloc(size_t bytes, size_t& nobjs)
	{
		size_t leftBytes = _endFree - _startFree;
		size_t totalBytes = bytes * nobjs;

		//Obj* MyFreeList;

		if (leftBytes >= totalBytes)  //内存池里的字节足够申请nobjs个n，去内存池里面切
		{
			__TRACE_DEBUG("内存池有足够%u个对象的内存\n", nobjs);
			//__TRACE_DEBUG("从内存池里面切%d块，每块%dbytes\n", nobjs, n);
			char* chunk = _startFree;
			_startFree = _startFree + totalBytes;
			return chunk;
		}
		else
			if (leftBytes >= bytes) // 内存池里的字节只够切1个及以上个n字节内存。
			{
				__TRACE_DEBUG("内存池只有%u个对象的内存\n", nobjs);
				nobjs = leftBytes / bytes;
				//__TRACE_DEBUG("从内存池里面切%d块，每块%dbytes\n", nobjs, n);
				totalBytes = nobjs * bytes;
				char* chunk = _startFree;
				_startFree += totalBytes;
				return chunk;
			}
			else // 内存池里面一个n字节大小的内存都没有
			{
				//1.尝试挂起剩余的小片内存

				if (leftBytes > 0)
				{
					size_t index = FREELIST_INDEX(leftBytes);
					((Obj*)_startFree)->_freeListLink = _freeList[index];
					_freeList[index] = (Obj*)_startFree;
				}
				//2.找系统申请
				size_t bytesToGet = totalBytes * 2 + (ROUND_UP(_heapsize >> 4));
				_startFree = (char*)malloc(bytesToGet);
				__TRACE_DEBUG("内存池没有内存，到系统申请%ubytes\n", bytesToGet);

				if (_startFree == NULL)
				{
					//到更大的自由链表中找内存块
					for (size_t i = FREELIST_INDEX(bytes); i < __NFREELISTS; i++)
					{

						if (_freeList[i])
						{
							_startFree = (char*)_freeList[i];
							_freeList[i] = ((Obj*)_startFree)->_freeListLink;
							_endFree = _startFree + (i + 1)*__ALIGN;

							return ChunkAlloc(bytes, nobjs);
						}
					}

					//最后一条路径，找一级空间配置器
					_endFree = 0;
					_startFree = (char*)__MallocAllocTemplate<0>::Allocate(bytesToGet);

				}
				_heapsize += bytesToGet;
				_endFree = _startFree + bytesToGet;
				return ChunkAlloc(bytes, nobjs);
			}
	}

	//填充函数
	static void* Refill(size_t n)
	{
		size_t nobjs = 20;//表示一次直接切20个n大小的内存块

		char* chunk = ChunkAlloc(n, nobjs);
		//先切nobjs个内存块，最少切一块，njobs是穿的引用

		if (nobjs == 1)
			return chunk;
		size_t index = FREELIST_INDEX(n);
		__TRACE_DEBUG("返回一个对象，将剩余的%u个对象挂到freeList[%u]下面\n", nobjs - 1, index);
		//返回一块，挂nobjs-1块
		//先挂第一块，再挂剩余的18块
		Obj* cur = (Obj*)(chunk + n);
	
		//_freeList是自由链表的首地址，此时找到cur挂的位置
		_freeList[index] = cur;

		//接着挂剩余的18块
		for (size_t i = 0; i < nobjs - 2; i++)
		{
			//找到cur后面的一块开始挂，此时已经挂的是第二块
			Obj* next = (Obj*)((char*)cur + n);
			cur->_freeListLink = next;

			//更新cur，让cur往后走
			cur = next;
		}
		cur->_freeListLink = NULL;
		return chunk;
	}
	static void* Allocate(size_t n)
	{
		__TRACE_DEBUG("二级空间配置器申请%ubytes\n", n);
		//大于128调用一级空配置器
		if (n > __MAX_BYTES)
		{
			
			return __MallocAllocTemplate<0>::Allocate(n);
		}

		size_t index = FREELIST_INDEX(n);

		if (_freeList[index] == NULL)
		{
			__TRACE_DEBUG("freeList[%u]下面没有内存块对象，需要到内存申请\n", index);
			//从内存池切
			return Refill(ROUND_UP(n));//填充函数
		}
		else
		{
			__TRACE_DEBUG("freeList[%u]取一个对象返回\n", index);
			//从自由链表中找
			Obj* ret = _freeList[index];
			_freeList[index] = ret->_freeListLink;
			return ret;
		}

	}
	static void Deallocate(void* p, size_t n)
	{
		if (n > __MAX_BYTES)
		{
			__MallocAllocTemplate<0>::Deallocate(p, n);
			return;
		}
		{
			size_t index = FREELIST_INDEX(n);
			__TRACE_DEBUG("二级空间配置器释放对象，挂到freeList[%u]下\n", index);
			Obj* obj = (Obj*)p;
			obj->_freeListLink = _freeList[index];
			_freeList[index] = obj;
		}
	}

};
template <bool threads, int inst>
char* __DefaultAllocTemplate<threads,inst>::_startFree = NULL;

template <bool threads, int inst>
char* __DefaultAllocTemplate<threads, inst>::_endFree = 0;

 template <bool threads, int inst>
 size_t __DefaultAllocTemplate<threads, inst>::_heapsize = 0;

//自由链表
 template <bool threads, int inst>
 typename __DefaultAllocTemplate<threads, inst>::Obj*
 __DefaultAllocTemplate<threads, inst>::_freeList[__NFREELISTS] = { 0 };

 void test2()
 {
	 void* p1=__DefaultAllocTemplate<false, 0>::Allocate(10);
	 //__DefaultAllocTemplate<false, 0>::Deallocate(p1, 6);
 }

#ifdef _USE_MALLOC
	 typedef __MallocAllocTemplate<0> alloc;
#else
	typedef __DefaultAllocTemplate<false,0> alloc;

#endif 

 template<class T, class Alloc>
 class SimpleAlloc {

 public:
	 static T* Allocate(size_t n)
	 {
		 return 0 == n ? 0 : (T*)Alloc::Allocate(n * sizeof(T));
	 }
	 static T* Allocate()
	 {
		 return (T*)Alloc::Allocate(sizeof(T));
	 }
	 static void Deallocate(T *p, size_t n)
	 {
		if (0 != n) Alloc::Deallocate(p, n * sizeof(T));
	 }
	 static void Deallocate(T *p)
	 {
		 Alloc::Deallocate(p, sizeof(T));
	 }
 };
