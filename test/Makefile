cpp=g++

target_name=exptest
target_inc_path=../inc/
target_out_path=./out/
target_obj_path=./out/obj/
target_dep_path=./out/dep/

cpp_flags=-Wall -fPIC -g -O3 -std=c++11 -D_REENTRANT -DNDEBUG -I $(target_inc_path)

shell=/bin/bash
all_dirs=$(shell ls -R | grep '^\./.*:$$' | awk '{gsub(":","");print}') .
target_src_files=$(foreach n,$(all_dirs), $(wildcard $(n)/*.cpp))
target_obj_files=$(patsubst %.cpp,$(target_obj_path)%.o,$(target_src_files))
target_dep_files=$(patsubst %.cpp,$(target_dep_path)%.d,$(target_src_files))

all : $(target_name)
	
$(target_name) : $(target_obj_files)
	$(cpp) $(cpp_flags) -o $@ $(target_obj_files)

$(target_dep_path)%.d : %.cpp
	@set -e; \
	rm -f $@; \
	$(cpp) -MM $(cpp_flags) $< > $@; \
	sed -i 's,\(.*\)\.o *:,\1.o $@:,g' $@

$(target_obj_path)%.o : %.cpp
	$(cpp) $(cpp_flags) -c -o "$@" "$<"

clean :
	rm -f $(target_obj_files) $(target_dep_files) $(target_out_path)$(target_name).so

clean-deps :
	rm -f $(target_dep_files)

ifneq "$(MAKECMDGOALS)" "clean"
-include $(target_dep_files)
endif
