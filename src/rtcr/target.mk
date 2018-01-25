TARGET = rtcr

SRC_CC += main.cc \
          cpu_session_component.cc \
          pd_session_component.cc \
          ram_session_component.cc \
          fault_handler.cc \
          region_map_component.cc \
          log_session.cc \
          rm_session.cc \
          timer_session.cc \
          target_child.cc
          
LIBS   += base

vpath cpu_session_component.cc $(PRG_DIR)/intercept
vpath pd_session_component.cc  $(PRG_DIR)/intercept
vpath ram_session_component.cc $(PRG_DIR)/intercept
vpath fault_handler.cc         $(PRG_DIR)/intercept
vpath region_map_component.cc  $(PRG_DIR)/intercept
vpath log_session.cc           $(PRG_DIR)/intercept
vpath rm_session.cc            $(PRG_DIR)/intercept
vpath timer_session.cc         $(PRG_DIR)/intercept
