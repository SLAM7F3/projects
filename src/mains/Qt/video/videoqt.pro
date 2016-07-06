CONFIG += qt opengl thread console debug_and_release
QT += opengl xml

TARGET = videoqt

TEMPLATE_TYPE = app

INCLUDEPATH += . \
	/home/cho/comp_vision/libcgt-src-win32-0.1/include

# LIBCGT
CONFIG( debug, debug|release ) {
    LIBS += -L/home/cho/comp_vision/libcgt-src-win32-0.1 -llibcgtd
} else {
    LIBS += -L/home/cho/comp_vision/libcgt-src-win32-0.1 -llibcgt
}

# Cg
LIBS += -lCg
LIBS += -lCgGL

# ffmpeg

LIBS += -lavformat
LIBS += -lavcodec
LIBS += -lavutil


# GLEW
LIBS += -lGLEW

HEADERS += src/AppData.h
HEADERS += src/ArrayIndexKernel.h
HEADERS += src/BilateralKernel.h
HEADERS += src/BufferObject2TextureKernel.h
HEADERS += src/CDFKernel.h
HEADERS += src/CombineQuantizedLuminanceAndDoGEdgesKernel.h
HEADERS += src/CombineTexturenessKernel.h
HEADERS += src/Controls.h
HEADERS += src/CrossBilateralKernel.h
HEADERS += src/DetailMapKernel.h
HEADERS += src/DifferenceOfGaussiansEdgesKernel.h
HEADERS += src/FilenameToRGBArrayKernel.h
HEADERS += src/FloatSliderUIData.h
HEADERS += src/GaussianBlurKernel.h
HEADERS += src/GazeBasedDetailAdjustmentKernel.h
HEADERS += src/GPUKernel.h
HEADERS += src/GridPaintingKernel.h
HEADERS += src/HistogramKernel.h
HEADERS += src/HistogramTransferKernel.h
HEADERS += src/InputKernelPort.h
HEADERS += src/InteractiveToneMapKernel.h
HEADERS += src/IntSliderUIData.h
HEADERS += src/KernelGraph.h
HEADERS += src/KernelPort.h
HEADERS += src/KernelPortData.h
HEADERS += src/KernelPortDataType.h
HEADERS += src/Lab2RGBKernel.h
HEADERS += src/LinearCombinationKernel.h
HEADERS += src/LocalHistogramEqualizationKernel.h
HEADERS += src/LuminanceQuantizationKernel.h
HEADERS += src/OutputKernelPort.h
HEADERS += src/OutputWidget.h
HEADERS += src/PerPixelProcessorKernel.h
HEADERS += src/QBilateralSigmaWidget.h
HEADERS += src/QSplineWidget.h
HEADERS += src/RGB2LabKernel.h
HEADERS += src/RGB2TextureKernel.h
HEADERS += src/RGB2VBOKernel.h
HEADERS += src/ScribbleKernel.h
HEADERS += src/StylizeKernel.h
HEADERS += src/Texture2BufferObjectKernel.h

SOURCES += src/AppData.cpp
SOURCES += src/ArrayIndexKernel.cpp
SOURCES += src/BilateralKernel.cpp
SOURCES += src/BufferObject2TextureKernel.cpp
SOURCES += src/CDFKernel.cpp
SOURCES += src/CombineQuantizedLuminanceAndDoGEdgesKernel.cpp
SOURCES += src/CombineTexturenessKernel.cpp
SOURCES += src/Controls.cpp
SOURCES += src/CrossBilateralKernel.cpp
SOURCES += src/DetailMapKernel.cpp
SOURCES += src/DifferenceOfGaussiansEdgesKernel.cpp
SOURCES += src/FilenameToRGBArrayKernel.cpp
SOURCES += src/GaussianBlurKernel.cpp
SOURCES += src/GazeBasedDetailAdjustmentKernel.cpp
SOURCES += src/GPUKernel.cpp
SOURCES += src/GridPaintingKernel.cpp
SOURCES += src/HistogramKernel.cpp
SOURCES += src/HistogramTransferKernel.cpp
SOURCES += src/InputKernelPort.cpp
SOURCES += src/InteractiveToneMapKernel.cpp
SOURCES += src/KernelGraph.cpp
SOURCES += src/KernelPort.cpp
SOURCES += src/KernelPortData.cpp
SOURCES += src/Lab2RGBKernel.cpp
SOURCES += src/LinearCombinationKernel.cpp
SOURCES += src/LocalHistogramEqualizationKernel.cpp
SOURCES += src/LuminanceQuantizationKernel.cpp
SOURCES += src/main.cpp
SOURCES += src/OutputKernelPort.cpp
SOURCES += src/OutputWidget.cpp
SOURCES += src/PerPixelProcessorKernel.cpp
SOURCES += src/QBilateralSigmaWidget.cpp
SOURCES += src/QSplineWidget.cpp
SOURCES += src/RGB2LabKernel.cpp
SOURCES += src/RGB2TextureKernel.cpp
SOURCES += src/RGB2VBOKernel.cpp
SOURCES += src/ScribbleKernel.cpp
SOURCES += src/StylizeKernel.cpp
SOURCES += src/Texture2BufferObjectKernel.cpp
