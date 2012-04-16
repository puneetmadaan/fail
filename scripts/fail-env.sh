#!/bin/bash
#
# fail-env.sh
# default values for several Fail* environment variables
# If you want to set your own defaults, or need a script to source from, e.g.,
# your ~/.bashrc, please copy this file and do not edit it in-place.
#

# A whitespace-separated list of hosts to rsync the experiment data to.  This
# is not necessarily the same list as FAIL_EXPERIMENT_HOSTS (see below), as
# many hosts may share their homes via NFS.
export FAIL_DISTRIBUTE_HOSTS=${FAIL_DISTRIBUTE_HOSTS:='ios kos virtuos plutonium bigbox.informatik.uni-erlangen.de'}

# A whitespace-separated list of hosts to run experiments on.  If the host name
# is followed by a ':' and a number, this specifies the number of clients to
# run on that host (defaults to #CPUs+1).
export FAIL_EXPERIMENT_HOSTS=${FAIL_EXPERIMENT_HOSTS:="bigbox.informatik.uni-erlangen.de plutonium uran virtuos ios:6 kos:6 bohrium polonium radon $(for hostnr in $(seq 100 254); do echo fiws$hostnr; done)"}

# A homedir-relative directory on the distribution hosts where all necessary
# Fail* ingredients reside (see multiple-clients.sh).
export FAIL_EXPERIMENT_TARGETDIR=${FAIL_EXPERIMENT_TARGETDIR:=.fail-experiment}

# Number of parallel build processes.  If unset, #CPUs+1.
if [ -z "$FAIL_BUILD_PARALLEL" ]; then
	if [ -e /proc/cpuinfo ]; then
		export FAIL_BUILD_PARALLEL=$(($(egrep -c ^processor /proc/cpuinfo)+1))
	else
		export FAIL_BUILD_PARALLEL=2
	fi
fi
