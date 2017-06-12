TARGET = rtcr_mc_checkpointer
#TARGET = rtcr_restore_child


SRC_CC += main.cc \
          pd_session.cc \
          cpu_session.cc \
          ram_session.cc \
          rom_session.cc \
          rm_session.cc \
          log_session.cc \
          timer_session.cc \
          cpu_thread_component.cc \
          region_map_component.cc \
          target_child.cc \
          target_state.cc \
          checkpointer.cc \
          restorer.cc

LIBS   += base

INC_DIR += $(BASE_DIR)/../base-foc/src/include

vpath pd_session.cc            $(REP_DIR)/src/rtcr/intercept
vpath cpu_session.cc           $(REP_DIR)/src/rtcr/intercept
vpath ram_session.cc           $(REP_DIR)/src/rtcr/intercept
vpath rom_session.cc           $(REP_DIR)/src/rtcr/intercept
vpath rm_session.cc            $(REP_DIR)/src/rtcr/intercept
vpath log_session.cc           $(REP_DIR)/src/rtcr/intercept
vpath timer_session.cc         $(REP_DIR)/src/rtcr/intercept
vpath cpu_thread_component.cc  $(REP_DIR)/src/rtcr/intercept
vpath region_map_component.cc  $(REP_DIR)/src/rtcr/intercept
vpath target_child.cc          $(REP_DIR)/src/rtcr
vpath target_state.cc          $(REP_DIR)/src/rtcr
vpath checkpointer.cc          $(REP_DIR)/src/rtcr
vpath restorer.cc              $(REP_DIR)/src/rtcr
