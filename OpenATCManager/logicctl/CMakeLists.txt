cmake_minimum_required(VERSION 3.0)


set(LOGICCTL_SRCS 
        LogicCtlActuate.cpp
        LogicCtlActuate.h
        LogicCtlActuateAdaptive.cpp
        LogicCtlActuateAdaptive.h
        LogicCtlActuatePedCrossStreet.cpp
        LogicCtlActuatePedCrossStreet.h
        LogicCtlAllRed.cpp
        LogicCtlAllRed.h
        LogicCtlCablelessLine.cpp
        LogicCtlCablelessLine.h
        LogicCtlChannelCheck.cpp
        LogicCtlChannelCheck.h
        LogicCtlFixedTime.cpp
        LogicCtlFixedTime.h
        LogicCtlLampOff.cpp
        LogicCtlLampOff.h
        LogicCtlManual.cpp
        LogicCtlManual.h
        LogicCtlMode.cpp
        LogicCtlMode.h
        LogicCtlModeSimpleFactory.cpp
        LogicCtlModeSimpleFactory.h
        LogicCtlOptim.cpp
        LogicCtlOptim.h
        LogicCtlPedCrossStreet.cpp
        LogicCtlPedCrossStreet.h
        LogicCtlWebsterOptim.cpp
        LogicCtlWebsterOptim.h
        LogicCtlYellowFlash.cpp
        LogicCtlYellowFlash.h
		LogicCtlPreempt.cpp
        LogicCtlPreempt.h
)

if(DEFINED VIRTUAL_DEVICE)
    set (LOGICCTL_SRCS ${LOGICCTL_SRCS}
        LogicCtlAdaptiveGreenWave.cpp
        LogicCtlAdaptiveGreenWave.h
        LogicCtlAIOptim.cpp
        LogicCtlAIOptim.h
        LogicCtlInductiveGreenWave.cpp
        LogicCtlInductiveGreenWave.h
    )
endif()

add_library(logicctl ${LOGICCTL_SRCS})
