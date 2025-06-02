declare module "xeue-gstreamer" {
	/**
	 * Joins an array of pipeline children together using the
	 * '!' separator.
	 *
	 * @param arr
	 * @returns
	 */
	export const pipelineFromArray = (arr: string[]) => string;

	type AppSink = {
		pull(callback: (buffer: any, caps: any) => void);

	}
	type AppSrc = {

	}

	type BussPollMessage = {
		type: string,
		[key: string]: any
	}

	class Pipeline {
		/**
		 * Creates a new GStreamer pipeline using the launch string provided
		 *
		 * @param launch The gstreamer launch pipeline text
		 *
		 *
		 * @example
		 * const pipeline = new gstreamer.Pipeline(
		 *  PipelineFromArray([
		 *   "videotestsrc pattern=ball",
		 *   "video/x-raw,width=320,height=240,framerate=100/1",
		 *   "theoraenc",
		 *   "oggmux",
		 *   "appsink max-buffers=1 name=sink"
		 * ])
		 * );

		 *
		 */
		constructor(launch: string);

		/**
		 * sets the pipeline state to `GST_STATE_PLAYING`
		 */
		play(): void;
		/**
		 *
		 */
		stop(): void;
		/**
		 *
		 */
		sendEOS(): void;
		/**
		 *
		 */
		pause(): void;

		/**
		 *
		 * @param time_nanoseconds
		 * @param flags TODO: Map this to a type
		 */
		seek(time_nanoseconds: number, flags: any)

		/**
		 *
		 */
		queryPosition(): number
		/**
		 *
		 */
		queryDuration(): number

		forceKeyUnit(sink: GObject, count: number)

		findChild(nodeName: string): GObject | AppSink | AppSrc

		setPad(element: GObject, attribute: string, padName: string)

		getPad(element: GObject, padName: string);

		pauseElement(element: GObject);
		playElement(element: GObject);
		negotiateElement(element: GObject);
		stopElement(element: GObject);

		pollBus(callback: (message: BussPollMessage) => void)

		rate(speed: number, flags: GstSeekFlags)

	}

	enum GstSeekFlags {
		/** No flag */
		NONE = 0,
		/** flush pipeline */
		FLUSH = 1 << 0,
		/** accurate position is requested, this might be considerably slower for some formats. */
		ACCURATE = 1 << 1,
		/** seek to the nearest keyframe. This might be faster but less accurate. */
		KEY_UNIT = 1 << 2,
		/** perform a segment seek. */
		SEGMENT = 1 << 3,
		/** when doing fast forward or fast reverse playback, allow elements to skip frames instead of generating all frames. (Since: 1.6) */
		TRICKMODE = 1 << 4,
		/** go to a location before the requested position, if KEY_UNIT this means the keyframe at or before the requested position the one at or before the seek target. */
		SNAP_BEFORE = 1 << 5,
		/** go to a location after the requested position, if KEY_UNIT this means the keyframe at of after the requested position. */
		SNAP_AFTER = 1 << 6,
		/** when doing fast forward or fast reverse playback, request that elements only decode keyframes and skip all other content, for formats that have keyframes. (Since: 1.6) */
		TRICKMODE_KEY_UNITS = 1 << 7,
		/** when doing fast forward or fast reverse playback, request that audio decoder elements skip decoding and output only gap events or silence. (Since: 1.6) */
		TRICKMODE_NO_AUDIO = 1 << 8,
		/** When doing fast forward or fast reverse playback, request that elements only decode keyframes and forward predicted frames and skip all other content (for example B-Frames), for formats that have keyframes and forward predicted frames. (Since: 1.18) */
		TRICKMODE_FORWARD_PREDICTED = 1 << 9,
		/** Signals that a rate change should be applied immediately. Only valid if start/stop position are GST_CLOCK_TIME_NONE, the playback direction does not change and the seek is not flushing. (Since: 1.18) */
		INSTANT_RATE_CHANGE = 1 << 10
	}
}
