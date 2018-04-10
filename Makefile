PKGNAME := com.mortehu.helloworld
VERSION_CODE := 1
VERSION_NAME := 0.1

KEYSTORE := $(HOME)/.android/debug.keystore
KEYSTORE_PASSWORD := android

ANDROID_JAR := $(HOME)/Android/Sdk/platforms/android-16/android.jar

MAIN_CLASS := \
  classes/com/mortehu/helloworld/HelloWorld.class

JAVA_SOURCES := \
  src/com/mortehu/helloworld/HelloWorld.java \
  src/com/mortehu/helloworld/OpenGLView.java

JNI_SOURCES := \
  jni/hello-world.cc

TARGET_APK := hello-world.apk

all: $(TARGET_APK)

$(MAIN_CLASS): $(JAVA_SOURCES)
	@mkdir -p classes/
	javac -source 1.7 -target 1.7 -cp src:$(ANDROID_JAR) -d classes/ $<

classes.dex: $(MAIN_CLASS)
	dalvik-exchange --dex --output $@ classes/

.DELETE_ON_ERROR:

lib/armeabi/libhello-world.so: $(JNI_SOURCES)
	NDK_LIBS_OUT=lib ndk-build APP_STL=gnustl_static

$(TARGET_APK).unaligned: classes.dex AndroidManifest.xml lib/armeabi/libhello-world.so
	aapt package -f -F $@ -M AndroidManifest.xml -I $(ANDROID_JAR) --rename-manifest-package $(PKGNAME) --version-code $(VERSION_CODE) --version-name $(VERSION_NAME) -c en
	aapt add -f $@ classes.dex
	find lib/ -type f -name \*.so | xargs -r aapt add -f $@

%.apk: %.apk.unaligned
	jarsigner -keystore $(KEYSTORE) -storepass $(KEYSTORE_PASSWORD) $< android-debug
	zipalign -f 4 $< $@

clean:
	rm -rf classes/ obj/ lib/
	rm -f $(TARGET_APK) $(TARGET_APK).unaligned

install: $(TARGET_APK)
	adb install -r $(TARGET_APK)

run:
	adb shell am start -a android.intent.action.MAIN -n com.mortehu.helloworld/.HelloWorld
