#ifndef __Pipeline_h__
#define __Pipeline_h__

#include <nan.h>
#include <gst/gst.h>

#include "GLibHelpers.h"

class Pipeline : public Nan::ObjectWrap {
	public:
		static void Init( Local<Object> exports );

		void play();
		void pause();
		void stop();
		gboolean seek(gint64 time_nanoseconds, GstSeekFlags flags);
		gboolean rate(double rate, GstSeekFlags flags);
		gint64 queryPosition();
		gint64 queryDuration();
		void sendEOS();
		void Quit();
		void forceKeyUnit(GObject* sink, int cnt);

		GObject *findChild( const char *name );
		Local<Value> pollBus();

		void setPad( GObject* elem, const char *attribute, const char *padName );
		GObject *getPad( GObject* elem, const char *padName );
		GObject *getPadCaps( GObject* elem, const char *padName );

		void setUpstreamProxy( GObject* elem, const char *sinkName, const char *srcName, GstElement* upstreamPipeline );
		void removeUpstreamProxy( GObject* elem, const char *srcName );
		
		void pauseElement( GObject* elem, const char *name );
		void playElement( GObject* elem, const char *name );
		void negotiateElement( GObject* elem, const char *name );
		void stopElement( GObject* elem, const char *name );
		

	private:
		Pipeline(const char *launch);
		Pipeline(GstPipeline *pipeline);
		~Pipeline();

		static Nan::Persistent<Function> constructor;

		GstPipeline *pipeline;

		static NAN_METHOD(New);
		static NAN_METHOD(Play);
		static NAN_METHOD(Pause);
		static NAN_METHOD(Stop);
		static NAN_METHOD(Seek);
		static NAN_METHOD(Rate);
		static NAN_METHOD(QueryPosition);
		static NAN_METHOD(QueryDuration);
		static NAN_METHOD(SendEOS);
		static NAN_METHOD(ForceKeyUnit);
		static NAN_METHOD(FindChild);
		static NAN_METHOD(SetPad);
		static NAN_METHOD(GetPad);
		static NAN_METHOD(GetPadCaps);
		static NAN_METHOD(SetUpstreamProxy);
		static NAN_METHOD(RemoveUpstreamProxy);
		static NAN_METHOD(PauseElement);
		static NAN_METHOD(PlayElement);
		static NAN_METHOD(NegotiateElement);
		static NAN_METHOD(StopElement);
		static NAN_METHOD(Quit);

		static void _doPollBus( uv_work_t *req );
		static void _polledBus( uv_work_t *req, int );
		static NAN_METHOD(PollBus);

		static NAN_GETTER(GetAutoFlushBus);
		static NAN_SETTER(SetAutoFlushBus);
		static NAN_GETTER(GetDelay);
		static NAN_SETTER(SetDelay);
		static NAN_GETTER(GetLatency);
		static NAN_SETTER(SetLatency);

};

#endif
