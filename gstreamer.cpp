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

// NODE_MODULE(gstreamer_superficial, init);

extern "C"
{
	static node::node_module _module = {127, 0, __null, "/home/sygnal/node-gstreamer-superficial/gstreamer.cpp", (node::addon_register_func)(init), __null, "gstreamer_superficial", __null, __null};
	static void _register_gstreamer_superficial(void) __attribute__((constructor));
	static void _register_gstreamer_superficial(void) { node_module_register(&_module); }
}