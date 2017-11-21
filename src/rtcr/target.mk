TARGET = rtcr

SRC_CC += main.cc \
          cpu_session.cc \
          pd_session.cc \
          ram_session.cc \
          region_map_component.cc \
          log_session.cc \
          rm_session.cc \
          timer_session.cc \
          target_child.cc
          
LIBS   += base

vpath cpu_session.cc $(REP_DIR)/include/rtcr/intercept
vpath pd_session.cc  $(REP_DIR)/include/rtcr/intercept
vpath ram_session.cc $(REP_DIR)/include/rtcr/intercept
vpath region_map_component.cc  $(REP_DIR)/include/rtcr/intercept
vpath log_session.cc           $(REP_DIR)/include/rtcr/intercept
vpath rm_session.cc            $(REP_DIR)/include/rtcr/intercept
vpath timer_session.cc         $(REP_DIR)/include/rtcr/intercept
