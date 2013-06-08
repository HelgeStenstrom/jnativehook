/* JNativeHook: Global keyboard and mouse hooking for Java.
 * Copyright (C) 2006-2013 Alexander Barker.  All Rights Received.
 * http://code.google.com/p/jnativehook/
 *
 * JNativeHook is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JNativeHook is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <jni.h>
#include "JNIGlobals.h"

// JNI Related global references.
JavaVM *jvm;
jint jni_version = JNI_VERSION_1_4;

// JNI entry point, This is executed when the Java virtual machine attaches to the native library.
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *UNUSED(reserved)) {
	/* Grab the currently running virtual machine so we can attach to it in
	 * functions that are not called from java. ( I.E. ThreadProc )
	 */
	jvm = vm;
	JNIEnv *env = NULL;
	if ((*jvm)->GetEnv(jvm, (void **)(&env), jni_version) == JNI_OK) {
		#ifdef DEBUG
		fprintf(stdout, "JNI_OnLoad(): GetEnv() successful.\n");
		#endif

		// Create all the global class references onload to prevent class loader
		// issues with JNLP and some IDE's.
		if (CreateJNIGlobals(env) != JNI_OK) {
			#ifdef DEBUG
			fprintf(stderr, "JNI_OnLoad(): CreateJNIGlobals() failed!\n");
			#endif

			ThrowFatalError("Failed to locate one or more required classes.");
		}

		// Set java properties from native sources.
		JNISetProperties(env);

		// Set the hook callback function to dispatch events.
		hook_set_dispatch_proc(&JNIEventDispatcher);
	}
	else {
		#ifdef DEBUG
		fprintf(stderr, "JNI_OnLoad(): GetEnv() failed!\n");
		#endif

		ThrowFatalError("Failed to aquire JNI interface pointer");
	}

	#ifdef DEBUG
	fprintf(stdout, "JNI_Load(): JNI Loaded.\n");
	#endif

    return jni_version;
}

// JNI exit point, This is executed when the Java virtual machine detaches from the native library.
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *UNUSED(vm), void *UNUSED(reserved)) {
	// Stop the native thread if its running.
	if (hook_is_enable()) {
		hook_disable();
	}

	// Grab the currently JNI interface pointer so we can cleanup the
	// system properties set on load.
	JNIEnv *env = NULL;
	if ((*jvm)->GetEnv(jvm, (void **)(&env), jni_version) == JNI_OK) {
		// Clear java properties from native sources.
		JNIClearProperties(env);
	}
	#ifdef DEBUG
	else {
		// It is not critical that these values are cleared so no exception
		// will be thrown.
		fprintf(stderr, "JNI_OnUnload(): GetEnv() failed!\n");
	}
	#endif

	JNIDestroyGlobals(env);

	#ifdef DEBUG
	fprintf(stdout, "JNI_OnUnload(): JNI Unloaded.\n");
	#endif
}