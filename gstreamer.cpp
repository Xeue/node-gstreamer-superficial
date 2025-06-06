#include <node.h>
#include <v8.h>

#include "GLibHelpers.h"
#include "GObjectWrap.h"
#include "Pipeline.h"

void init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE exports)
{
	gst_init(NULL, NULL);
	GObjectWrap::Init();
	Pipeline::Init(exports);
}

extern "C" NODE_MODULE_EXPORT void
NODE_MODULE_INITIALIZER(Local<Object> exports,
                        Local<Value> module,
                        Local<Context> context) {

	gst_init(NULL, NULL);
	GObjectWrap::Init();
	Pipeline::Init(exports);
}