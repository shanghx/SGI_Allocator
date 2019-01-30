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
