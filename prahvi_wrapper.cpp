#include "prahvi.hpp"
#include "prahvi_wrapper.h" 

#include <numpy/ndarrayobject.h>
#include <opencv2/core.hpp>

using namespace cv;

Mat fromNDArrayToMat(PyObject* o); 

extern "C" {
	prahvi* Prahvi_new(){ return new prahvi(); }
	ProcessResult Prahvi_getNewText(prahvi* pv, PyObject* img){ 
		cv::Mat cvImage = fromNDArrayToMat(img);

		return pv->getNewText(cvImage);
	}

	const char* Prahvi_getText(prahvi* pv) { return pv->getText().c_str(); }
}

//===================    MACROS    =================================================================
#define ERRWRAP2(expr) \
try \
{ \
    PyAllowThreads allowThreads; \
    expr; \
} \
catch (const cv::Exception &e) \
{ \
    PyErr_SetString(opencv_error, e.what()); \
    return 0; \
}
// The following conversion functions are taken from OpenCV's cv2.cpp file inside modules/python/src2 folder.
static PyObject* opencv_error = 0;

using namespace cv;
//===================   ERROR HANDLING     =========================================================

static int failmsg(const char *fmt, ...) {
	char str[1000];

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	PyErr_SetString(PyExc_TypeError, str);
	return 0;
}

static PyObject* failmsgp(const char *fmt, ...)
		{
	char str[1000];

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	PyErr_SetString(PyExc_TypeError, str);
	return 0;
}

//===================   THREADING     ==============================================================
class PyAllowThreads {
public:
	PyAllowThreads() :
			_state(PyEval_SaveThread()) {
	}
	~PyAllowThreads() {
		PyEval_RestoreThread(_state);
	}
private:
	PyThreadState* _state;
};

class PyEnsureGIL {
public:
	PyEnsureGIL() :
			_state(PyGILState_Ensure()) {
	}
	~PyEnsureGIL() {
		PyGILState_Release(_state);
	}
private:
	PyGILState_STATE _state;
};

enum {
	ARG_NONE = 0, ARG_MAT = 1, ARG_SCALAR = 2
};

class NumpyAllocator:
		public MatAllocator {
public:
	NumpyAllocator() {
		stdAllocator = Mat::getStdAllocator();
	}
	~NumpyAllocator() {
	}

	UMatData* allocate(PyObject* o, int dims, const int* sizes, int type,
			size_t* step) const {
		UMatData* u = new UMatData(this);
		u->data = u->origdata = (uchar*) PyArray_DATA((PyArrayObject*) o);
		npy_intp* _strides = PyArray_STRIDES((PyArrayObject*) o);
		for (int i = 0; i < dims - 1; i++)
			step[i] = (size_t) _strides[i];
		step[dims - 1] = CV_ELEM_SIZE(type);
		u->size = sizes[0] * step[0];
		u->userdata = o;
		return u;
	}

	UMatData* allocate(int dims0, const int* sizes, int type, void* data,
			size_t* step, int flags, UMatUsageFlags usageFlags) const {
		if (data != 0) {
			CV_Error(Error::StsAssert, "The data should normally be NULL!");
			// probably this is safe to do in such extreme case
			return stdAllocator->allocate(dims0, sizes, type, data, step, flags,
					usageFlags);
		}
		PyEnsureGIL gil;

		int depth = CV_MAT_DEPTH(type);
		int cn = CV_MAT_CN(type);
		const int f = (int) (sizeof(size_t) / 8);
		int typenum =
				depth == CV_8U ? NPY_UBYTE :
				depth == CV_8S ? NPY_BYTE :
				depth == CV_16U ? NPY_USHORT :
				depth == CV_16S ? NPY_SHORT :
				depth == CV_32S ? NPY_INT :
				depth == CV_32F ? NPY_FLOAT :
				depth == CV_64F ?
									NPY_DOUBLE :
									f * NPY_ULONGLONG + (f ^ 1) * NPY_UINT;
		int i, dims = dims0;
		cv::AutoBuffer<npy_intp> _sizes(dims + 1);
		for (i = 0; i < dims; i++)
			_sizes[i] = sizes[i];
		if (cn > 1)
			_sizes[dims++] = cn;
		PyObject* o = PyArray_SimpleNew(dims, _sizes, typenum);
		if (!o)
			CV_Error_(Error::StsError,
					("The numpy array of typenum=%d, ndims=%d can not be created", typenum, dims));
		return allocate(o, dims0, sizes, type, step);
	}

	bool allocate(UMatData* u, int accessFlags,
			UMatUsageFlags usageFlags) const {
		return stdAllocator->allocate(u, accessFlags, usageFlags);
	}

	void deallocate(UMatData* u) const {
		if (u) {
			PyEnsureGIL gil;
			PyObject* o = (PyObject*) u->userdata;
			Py_XDECREF(o);
			delete u;
		}
	}

	const MatAllocator* stdAllocator;
};

//===================   ALLOCATOR INITIALIZTION   ==================================================
NumpyAllocator g_numpyAllocator;

//===================   STANDALONE CONVERTER FUNCTIONS     =========================================

PyObject* fromMatToNDArray(const Mat& m) {
	if (!m.data)
		Py_RETURN_NONE;
		Mat temp,
	*p = (Mat*) &m;
	if (!p->u || p->allocator != &g_numpyAllocator) {
		temp.allocator = &g_numpyAllocator;
		ERRWRAP2(m.copyTo(temp));
		p = &temp;
	}
	PyObject* o = (PyObject*) p->u->userdata;
	Py_INCREF(o);
	return o;
}

Mat fromNDArrayToMat(PyObject* o) {
	cv::Mat m;
	bool allowND = true;
	if (false) {//!PyArray_Check(o)) {
		failmsg("argument is not a numpy array");
		if (!m.data)
			m.allocator = &g_numpyAllocator;
	} else {
		PyArrayObject* oarr = (PyArrayObject*) o;

		bool needcopy = false, needcast = false;
		int typenum = PyArray_TYPE(oarr), new_typenum = typenum;
		int type = typenum == NPY_UBYTE ? CV_8U : typenum == NPY_BYTE ? CV_8S :
					typenum == NPY_USHORT ? CV_16U :
					typenum == NPY_SHORT ? CV_16S :
					typenum == NPY_INT ? CV_32S :
					typenum == NPY_INT32 ? CV_32S :
					typenum == NPY_FLOAT ? CV_32F :
					typenum == NPY_DOUBLE ? CV_64F : -1;

		if (type < 0) {
			if (typenum == NPY_INT64 || typenum == NPY_UINT64
					|| type == NPY_LONG) {
				needcopy = needcast = true;
				new_typenum = NPY_INT;
				type = CV_32S;
			} else {
				failmsg("Argument data type is not supported");
				m.allocator = &g_numpyAllocator;
				return m;
			}
		}

#ifndef CV_MAX_DIM
		const int CV_MAX_DIM = 32;
#endif

		int ndims = PyArray_NDIM(oarr);
		if (ndims >= CV_MAX_DIM) {
			failmsg("Dimensionality of argument is too high");
			if (!m.data)
				m.allocator = &g_numpyAllocator;
			return m;
		}

		int size[CV_MAX_DIM + 1];
		size_t step[CV_MAX_DIM + 1];
		size_t elemsize = CV_ELEM_SIZE1(type);
		const npy_intp* _sizes = PyArray_DIMS(oarr);
		const npy_intp* _strides = PyArray_STRIDES(oarr);
		bool ismultichannel = ndims == 3 && _sizes[2] <= CV_CN_MAX;

		for (int i = ndims - 1; i >= 0 && !needcopy; i--) {
			// these checks handle cases of
			//  a) multi-dimensional (ndims > 2) arrays, as well as simpler 1- and 2-dimensional cases
			//  b) transposed arrays, where _strides[] elements go in non-descending order
			//  c) flipped arrays, where some of _strides[] elements are negative
			if ((i == ndims - 1 && (size_t) _strides[i] != elemsize)
					|| (i < ndims - 1 && _strides[i] < _strides[i + 1]))
				needcopy = true;
		}

		if (ismultichannel && _strides[1] != (npy_intp) elemsize * _sizes[2])
			needcopy = true;

		if (needcopy) {

			if (needcast) {
				o = PyArray_Cast(oarr, new_typenum);
				oarr = (PyArrayObject*) o;
			} else {
				oarr = PyArray_GETCONTIGUOUS(oarr);
				o = (PyObject*) oarr;
			}

			_strides = PyArray_STRIDES(oarr);
		}

		for (int i = 0; i < ndims; i++) {
			size[i] = (int) _sizes[i];
			step[i] = (size_t) _strides[i];
		}

		// handle degenerate case
		if (ndims == 0) {
			size[ndims] = 1;
			step[ndims] = elemsize;
			ndims++;
		}

		if (ismultichannel) {
			ndims--;
			type |= CV_MAKETYPE(0, size[2]);
		}

		if (ndims > 2 && !allowND) {
			failmsg("%s has more than 2 dimensions");
		} else {

			m = Mat(ndims, size, type, PyArray_DATA(oarr), step);
			m.u = g_numpyAllocator.allocate(o, ndims, size, type, step);
			m.addref();

			if (!needcopy) {
				Py_INCREF(o);
			}
		}
		m.allocator = &g_numpyAllocator;
	}
	return m;
}

//===================   BOOST CONVERTERS     =======================================================

//PyObject* matToNDArrayBoostConverter::convert(Mat const& m) {
//	if (!m.data)
//		Py_RETURN_NONE;
//		Mat temp,
//	*p = (Mat*) &m;
//	if (!p->u || p->allocator != &g_numpyAllocator)
//			{
//		temp.allocator = &g_numpyAllocator;
//		ERRWRAP2(m.copyTo(temp));
//		p = &temp;
//	}
//	PyObject* o = (PyObject*) p->u->userdata;
//	Py_INCREF(o);
//	return o;
//}
//
//matFromNDArrayBoostConverter::matFromNDArrayBoostConverter() {
//	boost::python::converter::registry::push_back(convertible, construct,
//			boost::python::type_id<Mat>());
//}
//
///// @brief Check if PyObject is an array and can be converted to OpenCV matrix.
//void* matFromNDArrayBoostConverter::convertible(PyObject* object) {
//	if (!PyArray_Check(object)) {
//		return NULL;
//	}
//#ifndef CV_MAX_DIM
//	const int CV_MAX_DIM = 32;
//#endif
//	PyArrayObject* oarr = (PyArrayObject*) object;
//
//	int typenum = PyArray_TYPE(oarr);
//	if (typenum != NPY_INT64 && typenum != NPY_UINT64 && typenum != NPY_LONG
//			&& typenum != NPY_UBYTE && typenum != NPY_BYTE
//			&& typenum != NPY_USHORT && typenum != NPY_SHORT
//			&& typenum != NPY_INT && typenum != NPY_INT32
//			&& typenum != NPY_FLOAT && typenum != NPY_DOUBLE) {
//		return NULL;
//	}
//	int ndims = PyArray_NDIM(oarr); //data type not supported
//
//	if (ndims >= CV_MAX_DIM) {
//		return NULL; //too many dimensions
//	}
//	return object;
//}
//
///// @brief Construct a Mat from an NDArray object.
//void matFromNDArrayBoostConverter::construct(PyObject* object,
//		boost::python::converter::rvalue_from_python_stage1_data* data) {
//	namespace python = boost::python;
//	// Object is a borrowed reference, so create a handle indicting it is
//	// borrowed for proper reference counting.
//	python::handle<> handle(python::borrowed(object));
//
//	// Obtain a handle to the memory block that the converter has allocated
//	// for the C++ type.
//	typedef python::converter::rvalue_from_python_storage<Mat> storage_type;
//	void* storage = reinterpret_cast<storage_type*>(data)->storage.bytes;
//
//	// Allocate the C++ type into the converter's memory block, and assign
//	// its handle to the converter's convertible variable.  The C++
//	// container is populated by passing the begin and end iterators of
//	// the python object to the container's constructor.
//	PyArrayObject* oarr = (PyArrayObject*) object;
//
//	bool needcopy = false, needcast = false;
//	int typenum = PyArray_TYPE(oarr), new_typenum = typenum;
//	int type = typenum == NPY_UBYTE ? CV_8U : typenum == NPY_BYTE ? CV_8S :
//				typenum == NPY_USHORT ? CV_16U :
//				typenum == NPY_SHORT ? CV_16S :
//				typenum == NPY_INT ? CV_32S :
//				typenum == NPY_INT32 ? CV_32S :
//				typenum == NPY_FLOAT ? CV_32F :
//				typenum == NPY_DOUBLE ? CV_64F : -1;
//
//	if (type < 0) {
//		needcopy = needcast = true;
//		new_typenum = NPY_INT;
//		type = CV_32S;
//	}
//
//#ifndef CV_MAX_DIM
//	const int CV_MAX_DIM = 32;
//#endif
//	int ndims = PyArray_NDIM(oarr);
//
//	int size[CV_MAX_DIM + 1];
//	size_t step[CV_MAX_DIM + 1];
//	size_t elemsize = CV_ELEM_SIZE1(type);
//	const npy_intp* _sizes = PyArray_DIMS(oarr);
//	const npy_intp* _strides = PyArray_STRIDES(oarr);
//	bool ismultichannel = ndims == 3 && _sizes[2] <= CV_CN_MAX;
//
//	for (int i = ndims - 1; i >= 0 && !needcopy; i--) {
//		// these checks handle cases of
//		//  a) multi-dimensional (ndims > 2) arrays, as well as simpler 1- and 2-dimensional cases
//		//  b) transposed arrays, where _strides[] elements go in non-descending order
//		//  c) flipped arrays, where some of _strides[] elements are negative
//		if ((i == ndims - 1 && (size_t) _strides[i] != elemsize)
//				|| (i < ndims - 1 && _strides[i] < _strides[i + 1]))
//			needcopy = true;
//	}
//
//	if (ismultichannel && _strides[1] != (npy_intp) elemsize * _sizes[2])
//		needcopy = true;
//
//	if (needcopy) {
//
//		if (needcast) {
//			object = PyArray_Cast(oarr, new_typenum);
//			oarr = (PyArrayObject*) object;
//		} else {
//			oarr = PyArray_GETCONTIGUOUS(oarr);
//			object = (PyObject*) oarr;
//		}
//
//		_strides = PyArray_STRIDES(oarr);
//	}
//
//	for (int i = 0; i < ndims; i++) {
//		size[i] = (int) _sizes[i];
//		step[i] = (size_t) _strides[i];
//	}
//
//	// handle degenerate case
//	if (ndims == 0) {
//		size[ndims] = 1;
//		step[ndims] = elemsize;
//		ndims++;
//	}
//
//	if (ismultichannel) {
//		ndims--;
//		type |= CV_MAKETYPE(0, size[2]);
//	}
//	if (!needcopy) {
//		Py_INCREF(object);
//	}
//
//	cv::Mat* m = new (storage) cv::Mat(ndims, size, type, PyArray_DATA(oarr), step);
//	m->u = g_numpyAllocator.allocate(object, ndims, size, type, step);
//	m->allocator = &g_numpyAllocator;
//	m->addref();
//	data->convertible = storage;
//} 

//static int failmsg(const char *fmt, ...)
//{
//    char str[1000];
//
//    va_list ap;
//    va_start(ap, fmt);
//    vsnprintf(str, sizeof(str), fmt, ap);
//    va_end(ap);
//
//    PyErr_SetString(PyExc_TypeError, str);
//    return 0;
//}
//
//class PyAllowThreads
//{
//public:
//    PyAllowThreads() : _state(PyEval_SaveThread()) {}
//    ~PyAllowThreads()
//    {
//        PyEval_RestoreThread(_state);
//    }
//private:
//    PyThreadState* _state;
//};
//
//class PyEnsureGIL
//{
//public:
//    PyEnsureGIL() : _state(PyGILState_Ensure()) {}
//    ~PyEnsureGIL()
//    {
//        PyGILState_Release(_state);
//    }
//private:
//    PyGILState_STATE _state;
//};
//
//#define ERRWRAP2(expr) \
//try \
//{ \
//    PyAllowThreads allowThreads; \
//    expr; \
//} \
//catch (const cv::Exception &e) \
//{ \
//    PyErr_SetString(opencv_error, e.what()); \
//    return 0; \
//}
//
//
//static PyObject* failmsgp(const char *fmt, ...)
//{
//  char str[1000];
//
//  va_list ap;
//  va_start(ap, fmt);
//  vsnprintf(str, sizeof(str), fmt, ap);
//  va_end(ap);
//
//  PyErr_SetString(PyExc_TypeError, str);
//  return 0;
//}
//
//static size_t REFCOUNT_OFFSET = (size_t)&(((PyObject*)0)->ob_refcnt) +
//    (0x12345678 != *(const size_t*)"\x78\x56\x34\x12\0\0\0\0\0")*sizeof(int);
//
//static inline PyObject* pyObjectFromRefcount(const int* refcount)
//{
//    return (PyObject*)((size_t)refcount - REFCOUNT_OFFSET);
//}
//
//static inline int* refcountFromPyObject(const PyObject* obj)
//{
//    return (int*)((size_t)obj + REFCOUNT_OFFSET);
//}
//
//class NumpyAllocator : public MatAllocator
//{
//public:
//    NumpyAllocator() {}
//    ~NumpyAllocator() {}
//
//    void allocate(int dims, const int* sizes, int type, int*& refcount,
//                  uchar*& datastart, uchar*& data, size_t* step)
//    {
//        PyEnsureGIL gil;
//
//        int depth = CV_MAT_DEPTH(type);
//        int cn = CV_MAT_CN(type);
//        const int f = (int)(sizeof(size_t)/8);
//        int typenum = depth == CV_8U ? NPY_UBYTE : depth == CV_8S ? NPY_BYTE :
//                      depth == CV_16U ? NPY_USHORT : depth == CV_16S ? NPY_SHORT :
//                      depth == CV_32S ? NPY_INT : depth == CV_32F ? NPY_FLOAT :
//                      depth == CV_64F ? NPY_DOUBLE : f*NPY_ULONGLONG + (f^1)*NPY_UINT;
//        int i;
//        npy_intp _sizes[CV_MAX_DIM+1];
//        for( i = 0; i < dims; i++ )
//        {
//            _sizes[i] = sizes[i];
//        }
//
//        if( cn > 1 )
//        {
//            /* if( _sizes[dims-1] == 1 )
//                _sizes[dims-1] = cn;
//            else */
//                _sizes[dims++] = cn;
//        }
//
//        PyObject* o = PyArray_SimpleNew(dims, _sizes, typenum);
//
//        if(!o)
//        {
//            CV_Error_(CV_StsError, ("The numpy array of typenum=%d, ndims=%d can not be created", typenum, dims));
//        }
//        refcount = refcountFromPyObject(o);
//
//        npy_intp* _strides = PyArray_STRIDES(o);
//        for( i = 0; i < dims - (cn > 1); i++ )
//            step[i] = (size_t)_strides[i];
//        datastart = data = (uchar*)PyArray_DATA(o);
//    }
//
//    void deallocate(int* refcount, uchar*, uchar*)
//    {
//        PyEnsureGIL gil;
//        if( !refcount )
//            return;
//        PyObject* o = pyObjectFromRefcount(refcount);
//        Py_INCREF(o);
//        Py_DECREF(o);
//    }
//};
//
//NumpyAllocator g_numpyAllocator;
//
//enum { ARG_NONE = 0, ARG_MAT = 1, ARG_SCALAR = 2 };
//
//static int pyopencv_to(const PyObject* o, Mat& m, const char* name = "<unknown>", bool allowND=true)
//{
//    //NumpyAllocator g_numpyAllocator;
//    if(!o || o == Py_None)
//    {
//        if( !m.data )
//            m.allocator = &g_numpyAllocator;
//        return true;
//    }
//
//    if( !PyArray_Check(o) )
//    {
//        failmsg("%s is not a numpy array", name);
//        return false;
//    }
//
//    int typenum = PyArray_TYPE(o);
//    int type = typenum == NPY_UBYTE ? CV_8U : typenum == NPY_BYTE ? CV_8S :
//               typenum == NPY_USHORT ? CV_16U : typenum == NPY_SHORT ? CV_16S :
//               typenum == NPY_INT || typenum == NPY_LONG ? CV_32S :
//               typenum == NPY_FLOAT ? CV_32F :
//               typenum == NPY_DOUBLE ? CV_64F : -1;
//
//    if( type < 0 )
//    {
//        failmsg("%s data type = %d is not supported", name, typenum);
//        return false;
//    }
//
//    int ndims = PyArray_NDIM(o);
//    if(ndims >= CV_MAX_DIM)
//    {
//        failmsg("%s dimensionality (=%d) is too high", name, ndims);
//        return false;
//    }
//
//    int size[CV_MAX_DIM+1];
//    size_t step[CV_MAX_DIM+1], elemsize = CV_ELEM_SIZE1(type);
//    const npy_intp* _sizes = PyArray_DIMS(o);
//    const npy_intp* _strides = PyArray_STRIDES(o);
//    bool transposed = false;
//
//    for(int i = 0; i < ndims; i++)
//    {
//        size[i] = (int)_sizes[i];
//        step[i] = (size_t)_strides[i];
//    }
//
//    if( ndims == 0 || step[ndims-1] > elemsize ) {
//        size[ndims] = 1;
//        step[ndims] = elemsize;
//        ndims++;
//    }
//
//    if( ndims >= 2 && step[0] < step[1] )
//    {
//        std::swap(size[0], size[1]);
//        std::swap(step[0], step[1]);
//        transposed = true;
//    }
//
//    if( ndims == 3 && size[2] <= CV_CN_MAX && step[1] == elemsize*size[2] )
//    {
//        ndims--;
//        type |= CV_MAKETYPE(0, size[2]);
//    }
//
//    if( ndims > 2 && !allowND )
//    {
//        failmsg("%s has more than 2 dimensions", name);
//        return false;
//    }
//
//    m = Mat(ndims, size, type, PyArray_DATA(o), step);
//
//    if( m.data )
//    {
//        m.refcount = refcountFromPyObject(o);
//        m.addref(); // protect the original numpy array from deallocation
//                    // (since Mat destructor will decrement the reference counter)
//    };
//    m.allocator = &g_numpyAllocator;
//
//    if( transposed )
//    {
//        Mat tmp;
//        tmp.allocator = &g_numpyAllocator;
//        transpose(m, tmp);
//        m = tmp;
//    }
//    return true;
//}
//
//static PyObject* pyopencv_from(const Mat& m)
//{
//    if( !m.data )
//        Py_RETURN_NONE;
//    Mat temp, *p = (Mat*)&m;
//    if(!p->refcount || p->allocator != &g_numpyAllocator)
//    {
//        temp.allocator = &g_numpyAllocator;
//        m.copyTo(temp);
//        p = &temp;
//    }
//    p->addref();
//    return pyObjectFromRefcount(p->refcount);
//}
