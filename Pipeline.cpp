#include <nan.h>

#include <gst/gst.h>
#include <gst/video/video.h>

#include "GLibHelpers.h"
#include "GObjectWrap.h"
#include "Pipeline.h"

Nan::Persistent<Function> Pipeline::constructor;

#define NANOS_TO_DOUBLE(nanos)(((double)nanos)/1000000000)
//FIXME: guint64 overflow
#define DOUBLE_TO_NANOS(secs)((guint64)(secs*1000000000))

Pipeline::Pipeline(const char *launch) {
	GError *err = NULL;

	pipeline = (GstPipeline*)GST_BIN(gst_parse_launch(launch, &err));
	if(err) {
		fprintf(stderr,"GstError: %s\n", err->message);
		Nan::ThrowError(err->message);
	}
}

Pipeline::Pipeline(GstPipeline* pipeline) {
	this->pipeline = pipeline;
}

Pipeline::~Pipeline() {
    if (pipeline) {
        gst_object_unref(GST_OBJECT(pipeline));
        pipeline = nullptr;
    }
}

void Pipeline::Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE exports) {
	Nan::HandleScope scope;

	Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(Pipeline::New);
	ctor->InstanceTemplate()->SetInternalFieldCount(1);
	ctor->SetClassName(Nan::New("Pipeline").ToLocalChecked());

	Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
	// Prototype
	Nan::SetPrototypeMethod(ctor, "play", Play);
	Nan::SetPrototypeMethod(ctor, "pause", Pause);
	Nan::SetPrototypeMethod(ctor, "stop", Stop);
	Nan::SetPrototypeMethod(ctor, "seek", Seek);
	Nan::SetPrototypeMethod(ctor, "rate", Rate);
	Nan::SetPrototypeMethod(ctor, "queryPosition", QueryPosition);
	Nan::SetPrototypeMethod(ctor, "queryDuration", QueryDuration);
	Nan::SetPrototypeMethod(ctor, "sendEOS", SendEOS);
	Nan::SetPrototypeMethod(ctor, "forceKeyUnit", ForceKeyUnit);
	Nan::SetPrototypeMethod(ctor, "findChild", FindChild);
	Nan::SetPrototypeMethod(ctor, "setPad", SetPad);
	Nan::SetPrototypeMethod(ctor, "getPad", GetPad);
	Nan::SetPrototypeMethod(ctor, "getPadCaps", GetPadCaps);
	Nan::SetPrototypeMethod(ctor, "setUpstreamProxy", SetUpstreamProxy);
	Nan::SetPrototypeMethod(ctor, "removeUpstreamProxy", RemoveUpstreamProxy);
	Nan::SetPrototypeMethod(ctor, "pauseElement", PauseElement);
	Nan::SetPrototypeMethod(ctor, "playElement", PlayElement);
	Nan::SetPrototypeMethod(ctor, "negotiateElement", NegotiateElement);
	Nan::SetPrototypeMethod(ctor, "stopElement", StopElement);
	Nan::SetPrototypeMethod(ctor, "pollBus", PollBus);
	Nan::SetPrototypeMethod(ctor, "quit", Quit);

	Nan::SetAccessor(proto, Nan::New("auto-flush-bus").ToLocalChecked(), GetAutoFlushBus, SetAutoFlushBus);
	Nan::SetAccessor(proto, Nan::New("delay").ToLocalChecked(), GetDelay, SetDelay);
	Nan::SetAccessor(proto, Nan::New("latency").ToLocalChecked(), GetLatency, SetLatency);

	constructor.Reset(Nan::GetFunction(ctor).ToLocalChecked());
	Nan::Set(exports, Nan::New("Pipeline").ToLocalChecked(), Nan::GetFunction(ctor).ToLocalChecked());
}

NAN_METHOD(Pipeline::New) {
	if (!info.IsConstructCall())
		return Nan::ThrowTypeError("Class constructors cannot be invoked without 'new'");

	Nan::Utf8String launch(info[0]);

	Pipeline* obj = new Pipeline(*launch);
	obj->Wrap(info.This());
	info.GetReturnValue().Set(info.This());
}

void Pipeline::play() {
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}

NAN_METHOD(Pipeline::Play) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	obj->play();
}

void Pipeline::stop() {
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
}

NAN_METHOD(Pipeline::Stop) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	obj->stop();
}

void Pipeline::sendEOS() {
	gst_element_send_event(GST_ELEMENT(pipeline), gst_event_new_eos());
}

NAN_METHOD(Pipeline::SendEOS) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	obj->sendEOS();
}

void Pipeline::pause() {
	gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PAUSED);
}

NAN_METHOD(Pipeline::Pause) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	obj->pause();
}

gboolean Pipeline::seek(gint64 time_seconds, GstSeekFlags flags) {
  return gst_element_seek (GST_ELEMENT(pipeline), 1.0, GST_FORMAT_TIME, flags,
                         GST_SEEK_TYPE_SET, time_seconds,
                         GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

NAN_METHOD(Pipeline::Seek) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	gint64 t(Nan::To<int64_t>(info[0]).ToChecked());
	bool useMs(Nan::To<bool>(info[2]).ToChecked());
	t *= useMs ? GST_MSECOND : GST_SECOND;
	GstSeekFlags flags((GstSeekFlags)Nan::To<Int32>(info[1]).ToLocalChecked()->Value());

	info.GetReturnValue().Set(Nan::New<Boolean>(obj->seek(t, flags)));
}

gboolean Pipeline::rate(double rate, GstSeekFlags flags) {
	gint64 start_pos;
	gst_element_query_position (GST_ELEMENT(pipeline), GST_FORMAT_TIME, &start_pos);
	if(rate > 0) {
		return gst_element_seek (GST_ELEMENT(pipeline), rate, GST_FORMAT_TIME, flags,
								GST_SEEK_TYPE_SET, start_pos,
								GST_SEEK_TYPE_END, 0);
	} else {
		return gst_element_seek (GST_ELEMENT(pipeline), rate, GST_FORMAT_TIME, flags,
								GST_SEEK_TYPE_SET, 0,
								GST_SEEK_TYPE_SET, start_pos);
	}
}

NAN_METHOD(Pipeline::Rate) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	double r(Nan::To<double>(info[0]).ToChecked());
	GstSeekFlags flags((GstSeekFlags)Nan::To<Int32>(info[1]).ToLocalChecked()->Value());

	info.GetReturnValue().Set(Nan::New<Boolean>(obj->rate(r,flags)));
}

gint64 Pipeline::queryPosition() {
  gint64 pos;
  gst_element_query_position (GST_ELEMENT(pipeline), GST_FORMAT_TIME, &pos);
  return pos;
}

NAN_METHOD(Pipeline::QueryPosition) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	gint64 t = obj->queryPosition();
	bool useMs(Nan::To<bool>(info[0]).ToChecked());
	double r = t==-1 ? -1 : (double)t/(useMs ? GST_MSECOND : GST_SECOND);

	info.GetReturnValue().Set(Nan::New<Number>(r));
}

gint64 Pipeline::queryDuration() {
  gint64 dur;
  gst_element_query_duration (GST_ELEMENT(pipeline), GST_FORMAT_TIME, &dur);
  return dur;
}

NAN_METHOD(Pipeline::QueryDuration) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	gint64 t = obj->queryDuration();
	bool useMs(Nan::To<bool>(info[1]).ToChecked());
	double r = t==-1 ? -1 : (double)t/(useMs ? GST_MSECOND : GST_SECOND);

	info.GetReturnValue().Set(Nan::New<Number>(r));
}

void Pipeline::forceKeyUnit(GObject *sink, int cnt) {
	GstPad *sinkpad = gst_element_get_static_pad(GST_ELEMENT(sink), "sink");
	gst_pad_push_event(sinkpad,(GstEvent*) gst_video_event_new_upstream_force_key_unit(GST_CLOCK_TIME_NONE, TRUE, cnt));
}

NAN_METHOD(Pipeline::ForceKeyUnit) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	Nan::Utf8String name(info[0]);
	GObject *o = obj->findChild(*name);
	int cnt(Nan::To<Int32>(info[1]).ToLocalChecked()->Value());
	obj->forceKeyUnit(o, cnt);
	info.GetReturnValue().Set(Nan::True());
}

GObject * Pipeline::findChild(const char *name) {
	GstElement *e = gst_bin_get_by_name(GST_BIN(pipeline), name);
	return G_OBJECT(e);
}

NAN_METHOD(Pipeline::FindChild) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	Nan::Utf8String name(info[0]);
	GObject *o = obj->findChild(*name);
	if(o)
		info.GetReturnValue().Set(GObjectWrap::NewInstance(info, o));
	else
		info.GetReturnValue().Set(Nan::Undefined());
}

void Pipeline::setPad(GObject *elem, const char *attribute, const char *padName) {
	GstPad *pad = gst_element_get_static_pad(GST_ELEMENT(elem), padName);
	if (!pad) {
		return;
	}
	g_object_set(elem, attribute, pad, NULL);
}

NAN_METHOD(Pipeline::SetPad) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	Nan::Utf8String name(info[0]);
	GObject *o = obj->findChild(*name);
	if(!o) {
		return;
	}

	Nan::Utf8String attribute(info[1]);
	Nan::Utf8String padName(info[2]);
	obj->setPad(o, *attribute, *padName);
}

GObject * Pipeline::getPad(GObject* elem, const char *padName ) {
	GstPad *pad = gst_element_get_static_pad(GST_ELEMENT(elem), padName);
	return G_OBJECT(pad);
}

NAN_METHOD(Pipeline::GetPad) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	Nan::Utf8String name(info[0]);
	GObject *o = obj->findChild(*name);
	if (!o) {
		return;
	}

	Nan::Utf8String padName(info[1]);
	GObject *pad = obj->getPad(o, *padName);
	if(pad) {
		info.GetReturnValue().Set(GObjectWrap::NewInstance(info, pad));
	} else {
		info.GetReturnValue().Set(Nan::Undefined());
	}
}

GObject * Pipeline::getPadCaps(GObject* elem, const char *padName ) {
	GstPad *pad = gst_element_get_static_pad(GST_ELEMENT(elem), padName);
	GstCaps *caps = gst_pad_get_current_caps(pad);
	return G_OBJECT(caps);
}

NAN_METHOD(Pipeline::GetPadCaps) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	Nan::Utf8String name(info[0]);
	GObject *o = obj->findChild(*name);
	if (!o) {
		return;
	}

	Nan::Utf8String padName(info[1]);
	GObject *caps = obj->getPadCaps(o, *padName);
	if(caps) {
		info.GetReturnValue().Set(GObjectWrap::NewInstance(info, caps));
	} else {
		info.GetReturnValue().Set(Nan::Undefined());
	}
}

NAN_METHOD(Pipeline::SetUpstreamProxy) {
	GObject *psink, *psrc;
	GstClock *clock;

	// Pipeline with proxysrc
	Pipeline* srcPipeline = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	// Pipeline with proxysink
	Nan::MaybeLocal<v8::Object> maybeSinkPipeline = Nan::To<v8::Object>(info[0]);
	Pipeline* sinkPipeline = Nan::ObjectWrap::Unwrap<Pipeline>(maybeSinkPipeline.ToLocalChecked());

	// sink
	Nan::Utf8String proxySinkName(info[1]);
	psink = sinkPipeline->findChild(*proxySinkName);
	// src
	Nan::Utf8String proxySrcName(info[2]);
	psrc = srcPipeline->findChild(*proxySrcName);

	// Connect the pipelines
	g_object_set (psrc, "proxysink", NULL, NULL);
	g_object_set (psrc, "proxysink", psink, NULL);

	// Sync the clock
	clock = gst_system_clock_obtain ();
	gst_pipeline_use_clock (GST_PIPELINE (srcPipeline->pipeline), clock);
	gst_pipeline_use_clock (GST_PIPELINE (sinkPipeline->pipeline), clock);
	g_object_unref (clock);
}

NAN_METHOD(Pipeline::RemoveUpstreamProxy) {
	GObject *psrc;

	// Pipeline with proxysrc
	Pipeline* srcPipeline = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	// proxysrc name
	Nan::Utf8String proxySrcName(info[0]);
	psrc = srcPipeline->findChild(*proxySrcName);

	// Remove connection
	g_object_set (psrc, "proxysink", NULL, NULL);
}

NAN_METHOD(Pipeline::PauseElement) {
	GstElement *e;
	Pipeline* pipeline = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	Nan::Utf8String name(info[0]);
	e = GST_ELEMENT(pipeline->findChild(*name));

	if(!e) {
		g_print("Element to pause not found\n");
		return;
	}
	gst_element_set_state (e, GST_STATE_PAUSED);
	g_print("Paused element\n");
}

NAN_METHOD(Pipeline::PlayElement) {
	GstElement *e;
	Pipeline* pipeline = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	Nan::Utf8String name(info[0]);
	e = GST_ELEMENT(pipeline->findChild(*name));

	if(!e) {
		g_print("Element to play not found\n");
		return;
	}

	// Reconfigure
	/*GstPad *srcpad = gst_element_get_static_pad(e, "src");
	gst_pad_push_event(srcpad, gst_event_new_reconfigure());
	g_print("Sent reconfigure event\n");*/

	// Play
	gst_element_set_state (e, GST_STATE_PLAYING);
	g_print("Played element\n");
}

NAN_METHOD(Pipeline::NegotiateElement) {
	GstElement *e;
	Pipeline* pipeline = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	Nan::Utf8String name(info[0]);
	e = GST_ELEMENT(pipeline->findChild(*name));

	if(!e) {
		g_print("Element to play not found\n");
		return;
	}

	// Reconfigure
	GstPad *srcpad = gst_element_get_static_pad(e, "src");
	gst_pad_push_event(srcpad, gst_event_new_reconfigure());
	g_print("Sent reconfigure event\n");

	// Play
	gst_element_set_state (e, GST_STATE_PLAYING);
	g_print("Played element\n");
}

NAN_METHOD(Pipeline::StopElement) {
	GstElement *e;
	Pipeline* pipeline = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	Nan::Utf8String name(info[0]);
	e = GST_ELEMENT(pipeline->findChild(*name));

	if(!e) {
		g_print("Element to stop not found\n");
		return;
	}
	gst_element_set_state (e, GST_STATE_NULL);
	g_print("Stopped element\n");
}

void Pipeline::Quit() {
	gst_object_unref(GST_ELEMENT(pipeline));
}

NAN_METHOD(Pipeline::Quit) {
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	if(!obj) {
		return;
	}

	obj->Quit();
}

class BusRequest : public Nan::AsyncResource {
	public:
		BusRequest(Pipeline *obj_, Local<Function> cb)
			: AsyncResource("node-gst-superficial.BusRequest")
			, obj(obj_)
			{
				callback.Reset(cb);;
				request.data = this;
			}

		~BusRequest() {
			callback.Reset();
		}

	uv_work_t request;
	Nan::Persistent<Function> callback;
	Pipeline *obj;
	GstMessage *msg;
};

void Pipeline::_doPollBus(uv_work_t *req) {
	BusRequest *br = static_cast<BusRequest*>(req->data);
	GstBus *bus = gst_element_get_bus(GST_ELEMENT(br->obj->pipeline));
	if(!bus) return;
	br->msg = gst_bus_timed_pop(bus, 200000000); // GST_CLOCK_TIME_NONE, 100000000 = 100 ms	
}

void Pipeline::_polledBus(uv_work_t *req, int n) {
	BusRequest *br = static_cast<BusRequest*>(req->data);

	if(br->msg) {
		Nan::HandleScope scope;
		Local<Object> m = Nan::New<Object>();
		Nan::Set(m, Nan::New("type").ToLocalChecked(), Nan::New(GST_MESSAGE_TYPE_NAME(br->msg)).ToLocalChecked());

		const GstStructure *structure = (GstStructure*)gst_message_get_structure(br->msg);
		if(structure)
			gst_structure_to_v8(m, structure);

		GstObject *src = br->msg->src;
		Local<String> v = Nan::New<String>((const char *)src->name).ToLocalChecked();
		Nan::Set(m, Nan::New("_src_element_name").ToLocalChecked(), v);

		if(GST_MESSAGE_TYPE(br->msg) == GST_MESSAGE_ERROR) {
			GError *err = NULL;
			gchar *name = gst_object_get_path_string(br->msg->src);
			Nan::Set(m,Nan::New("name").ToLocalChecked(), Nan::New(name).ToLocalChecked());
			gst_message_parse_error(br->msg, &err, NULL);
			Nan::Set(m,Nan::New("message").ToLocalChecked(), Nan::New(err->message).ToLocalChecked());
		}

		Local<Value> argv[1] = { m };

		v8::Local<v8::Function> callback = Nan::New(br->callback);
//		br->callback.Call(1, argv);
		v8::Local<v8::Object> target = Nan::New<v8::Object>();
		br->runInAsyncScope(target, callback, 1, argv);

		gst_message_unref(br->msg);
	}
	if(GST_STATE(br->obj->pipeline) != GST_STATE_NULL)
		uv_queue_work(Nan::GetCurrentEventLoop(), &br->request, _doPollBus, _polledBus);
}

NAN_METHOD(Pipeline::PollBus) {
	Nan::EscapableHandleScope scope;
	Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());

	if(info.Length() == 0 || !info[0]->IsFunction()) {
		Nan::ThrowError("Callback is required and must be a Function.");
		scope.Escape(Nan::Undefined());
		return;
	}

	Local<Function> callback = Local<Function>::Cast(info[0]);

	BusRequest * br = new BusRequest(obj,callback);
	obj->Ref();
	uv_queue_work(Nan::GetCurrentEventLoop(), &br->request, _doPollBus, _polledBus);

	scope.Escape(Nan::Undefined());
}


NAN_GETTER(Pipeline::GetAutoFlushBus) {
	Pipeline *obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	info.GetReturnValue().Set(Nan::New<Boolean>(gst_pipeline_get_auto_flush_bus(obj->pipeline)));
}

NAN_SETTER(Pipeline::SetAutoFlushBus) {
	if(value->IsBoolean()) {
		Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
		gst_pipeline_set_auto_flush_bus(obj->pipeline, Nan::To<Boolean>(value).ToLocalChecked()->Value());
	}
}

NAN_GETTER(Pipeline::GetDelay) {
	Pipeline *obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	double secs = NANOS_TO_DOUBLE(gst_pipeline_get_delay(obj->pipeline));
	info.GetReturnValue().Set(Nan::New<Number>(secs));
}

NAN_SETTER(Pipeline::SetDelay) {
	if(value->IsNumber()) {
		Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
		gst_pipeline_set_delay(obj->pipeline, DOUBLE_TO_NANOS(Nan::To<Number>(value).ToLocalChecked()->Value()));
	}
}
NAN_GETTER(Pipeline::GetLatency) {
	Pipeline *obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
	double secs = NANOS_TO_DOUBLE(gst_pipeline_get_latency(obj->pipeline));
	info.GetReturnValue().Set(Nan::New<Number>(secs));
}
NAN_SETTER(Pipeline::SetLatency) {
	if(value->IsNumber()) {
		Pipeline* obj = Nan::ObjectWrap::Unwrap<Pipeline>(info.This());
		gst_pipeline_set_latency(obj->pipeline, DOUBLE_TO_NANOS(Nan::To<Number>(value).ToLocalChecked()->Value()));
	}
}
