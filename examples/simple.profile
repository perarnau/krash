cgroup_root = /
all_name = alltasks
cpu {				# cpu injection part
	profile {			# the load profile itself
		0 {				# cpuid of the cpu to load
			0	70		# MANDATORY : time 00:00:00 must be present for each cpu. From the beginning of the injection we load 70% of the cpu.
			60	30		# after one minute we load only 30% the cpu
			120	50
		}
		1 {				# another cpu loaded
			0	70
			10	30
		}
	}				# end of profile
}					# end of cpu injection part
kill 150
