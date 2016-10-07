TARGET = target_copy-tester

SRC_CC += main.cc \
          cpu_session_component.cc \
          pd_session_component.cc \
          ram_session_component.cc \
          region_map_component.cc \
          log_session.cc \
          rm_session.cc \
          timer_session.cc \
          target_child.cc \
          target_copy.cc

LIBS   += base

vpath cpu_session_component.cc $(REP_DIR)/src/rtcr/intercept
vpath pd_session_component.cc  $(REP_DIR)/src/rtcr/intercept
vpath ram_session_component.cc $(REP_DIR)/src/rtcr/intercept
vpath region_map_component.cc  $(REP_DIR)/src/rtcr/intercept
vpath log_session.cc           $(REP_DIR)/src/rtcr/intercept
vpath rm_session.cc            $(REP_DIR)/src/rtcr/intercept
vpath timer_session.cc         $(REP_DIR)/src/rtcr/intercept
vpath target_child.cc          $(REP_DIR)/src/rtcr
vpath target_copy.cc           $(REP_DIR)/src/rtcr
