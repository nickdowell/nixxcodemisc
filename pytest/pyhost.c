
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>

#include <Python/Python.h>

#define SIZEOF_STATIC_ARRAY( a ) ( sizeof((a)) / sizeof((a)[0]) )

char *pythonArgv[] = {
	"arg0",
	"argument 1",
	"something else",
	"forth",
};

const char *pythonScript =
"import sys\n"
"print sys.argv\n"
;

#if 1

// **** THE EASY WAY :-) ****

int main()
{
	Py_Initialize();
	PySys_SetArgv(SIZEOF_STATIC_ARRAY(pythonArgv), pythonArgv);
	PyRun_SimpleStringFlags(pythonScript, NULL);
	Py_Finalize();
	return 0;
}

#else

// **** THE HARD WAY :-\ ****

#define PYTHONLIB_PATH "/System/Library/Frameworks/Python.framework/Python"

typedef struct
{
	void (* Py_Initialize)(void);
	void (* Py_Finalize)(void);
	void (* PySys_SetArgv)(int, char **);
	void (* PySys_SetPath)(char *);
	int  (* PyRun_SimpleStringFlags)(const char *, PyCompilerFlags *);
}
python_functions_t;

int main()
{
	//
	// WARNING: Should use NSCreateObjectFileImageFromFile instead
	// http://developer.apple.com/mac/library/technotes/tn2002/tn2071.html#Section6
	//
	void *pythonLib = dlopen(PYTHONLIB_PATH, RTLD_NOW);
	if (!pythonLib) {
		char *error = dlerror();
		fprintf(stderr, "dlopen() failed: %s\n", error);
		return 1;
	}
	
	python_functions_t python = {0};
#define LOAD_SYMBOL( symname ) if (!(python.symname = dlsym( pythonLib, #symname ))) { return -1; }
	LOAD_SYMBOL( Py_Initialize );
	LOAD_SYMBOL( Py_Finalize );
	LOAD_SYMBOL( PySys_SetArgv );
	LOAD_SYMBOL( PySys_SetPath );
	LOAD_SYMBOL( PyRun_SimpleStringFlags );
	
	python.Py_Initialize();
	python.PySys_SetArgv(SIZEOF_STATIC_ARRAY(pythonArgv), pythonArgv);
	python.PyRun_SimpleStringFlags(pythonScript, NULL);
	python.Py_Finalize();
	
	dlclose(pythonLib);
	
	return 0;
}

#endif

