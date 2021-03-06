#include "xbase/x_target.h"
#include "xbase/x_types.h"
#include "xbase/x_allocator.h"
#include "xunittest/xunittest.h"

#include "xentry/x_entry.h"

UNITTEST_SUITE_LIST(xEntryUnitTest);
UNITTEST_SUITE_DECLARE(xEntryUnitTest, entry);

namespace xcore
{
	class TestHeapAllocator : public x_iallocator
	{
	public:
		TestHeapAllocator(xcore::x_iallocator* allocator)
			: mAllocator(allocator)
			, mNumAllocations(0)
		{
		}

		xcore::x_iallocator*	mAllocator;
		s32						mNumAllocations;

		virtual const char*	name() const
		{
			return "xentry unittest test heap allocator";
		}

		virtual void*		allocate(u32 size, u32 alignment)
		{
			++mNumAllocations;
			return mAllocator->allocate(size, alignment);
		}

		virtual void*		reallocate(void* mem, u32 size, u32 alignment)
		{
			return mAllocator->reallocate(mem, size, alignment);
		}

		virtual void		deallocate(void* mem)
		{
			--mNumAllocations;
			mAllocator->deallocate(mem);
		}

		virtual void		release()
		{
		}
	};
}

class UnitTestAllocator : public UnitTest::Allocator
{
public:
	xcore::x_iallocator*	mAllocator;
	int						mNumAllocations;

	UnitTestAllocator(xcore::x_iallocator* allocator)
		: mNumAllocations(0)
	{
		mAllocator = allocator;
	}

	virtual void*	Allocate(int size)
	{
		++mNumAllocations;
		return mAllocator->allocate(size, 4);
	}
	virtual void	Deallocate(void* ptr)
	{
		--mNumAllocations;
		mAllocator->deallocate(ptr);
	}
};

xcore::x_iallocator* gSystemAllocator = NULL;

bool gRunUnitTest(UnitTest::TestReporter& reporter)
{
	gSystemAllocator = xcore::gCreateSystemAllocator();
	UnitTestAllocator unittestAllocator( gSystemAllocator );
	UnitTest::SetAllocator(&unittestAllocator);

	xcore::TestHeapAllocator libHeapAllocator(gSystemAllocator);
	
	int r = UNITTEST_SUITE_RUN(reporter, xEntryUnitTest);
	if (unittestAllocator.mNumAllocations!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "xunittest", "memory leaks detected!");
		r = -1;
	}
	if (libHeapAllocator.mNumAllocations!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "xentry", "memory leaks detected!");
		r = -1;
	}

	UnitTest::SetAllocator(NULL);
	gSystemAllocator->release();
	gSystemAllocator = NULL;

	return r==0;
}

