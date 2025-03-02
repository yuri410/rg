# CMake build rules for the OCaml language.
# Assumes FindOCaml is used.
# http://ocaml.org/
#
# Example usage:
#
# add_ocaml_library(pkg_a OCAML mod_a OCAMLDEP pkg_b C mod_a_stubs PKG ctypes LLVM core)
#
# Unnamed parameters:
#
#   * Library name.
#
# Named parameters:
#
# OCAML     OCaml module names. Imply presence of a corresponding .ml and .mli files.
# OCAMLDEP  Names of libraries this library depends on.
# C         C stub sources. Imply presence of a corresponding .c file.
# CFLAGS    Additional arguments passed when compiling C stubs.
# PKG       Names of ocamlfind packages this library depends on.
# LLVM      Names of LLVM libraries this library depends on.
# NOCOPY    Do not automatically copy sources (.c, .ml, .mli) from the source directory,
#           e.g. if they are generated.
#

function(add_ocaml_library name)
  CMAKE_PARSE_ARGUMENTS(ARG "NOCOPY" "" "OCAML;OCAMLDEP;C;CFLAGS;PKG;LLVM" ${ARGN})

  set(src ${CMAKE_CURRENT_SOURCE_DIR})
  set(bin ${CMAKE_CURRENT_BINARY_DIR})

  set(ocaml_pkgs)
  foreach( ocaml_pkg ${ARG_PKG} )
    list(APPEND ocaml_pkgs "-package" "${ocaml_pkg}")
  endforeach()

  set(sources)

  set(ocaml_inputs)

  set(ocaml_outputs "${bin}/${name}.cma")
  if( ARG_C )
    list(APPEND ocaml_outputs
         "${bin}/lib${name}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    if ( BUILD_SHARED_LIBS )
      list(APPEND ocaml_outputs
           "${bin}/dll${name}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    endif()
  endif()
  if( HAVE_OCAMLOPT )
    list(APPEND ocaml_outputs
         "${bin}/${name}.cmxa"
         "${bin}/${name}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  endif()

  set(ocaml_flags "-lstdc++" "-ldopt" "-L${LLVM_LIBRARY_DIR}"
                  "-ccopt" "-L\\$CAMLORIGIN/.."
                  "-ccopt" "-Wl,-rpath,\\$CAMLORIGIN/.."
                  ${ocaml_pkgs})

  foreach( ocaml_dep ${ARG_OCAMLDEP} )
    get_target_property(dep_ocaml_flags "ocaml_${ocaml_dep}" OCAML_FLAGS)
    list(APPEND ocaml_flags ${dep_ocaml_flags})
  endforeach()

  if( NOT BUILD_SHARED_LIBS )
    list(APPEND ocaml_flags "-custom")
  endif()

  explicit_map_components_to_libraries(llvm_libs ${ARG_LLVM})
  foreach( llvm_lib ${llvm_libs} )
    list(APPEND ocaml_flags "-l${llvm_lib}" )
  endforeach()

  get_property(system_libs TARGET LLVMSupport PROPERTY LLVM_SYSTEM_LIBS)
  foreach(system_lib ${system_libs})
    list(APPEND ocaml_flags "-l${system_lib}" )
  endforeach()

  string(REPLACE ";" " " ARG_CFLAGS "${ARG_CFLAGS}")
  set(c_flags "${ARG_CFLAGS} ${LLVM_DEFINITIONS}")
  foreach( include_dir ${LLVM_INCLUDE_DIR} ${LLVM_MAIN_INCLUDE_DIR} )
    set(c_flags "${c_flags} -I${include_dir}")
  endforeach()

  foreach( ocaml_file ${ARG_OCAML} )
    list(APPEND sources "${ocaml_file}.mli" "${ocaml_file}.ml")

    list(APPEND ocaml_inputs "${bin}/${ocaml_file}.mli" "${bin}/${ocaml_file}.ml")

    list(APPEND ocaml_outputs "${bin}/${ocaml_file}.cmi" "${bin}/${ocaml_file}.cmo")
    if( HAVE_OCAMLOPT )
      list(APPEND ocaml_outputs
           "${bin}/${ocaml_file}.cmx"
           "${bin}/${ocaml_file}${CMAKE_C_OUTPUT_EXTENSION}")
    endif()
  endforeach()

  foreach( c_file ${ARG_C} )
    list(APPEND sources "${c_file}.c")

    list(APPEND c_inputs  "${bin}/${c_file}.c")
    list(APPEND c_outputs "${bin}/${c_file}${CMAKE_C_OUTPUT_EXTENSION}")
  endforeach()

  if( NOT ARG_NOCOPY )
    foreach( source ${sources} )
      add_custom_command(
          OUTPUT "${bin}/${source}"
          COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${src}/${source}" "${bin}"
          DEPENDS "${src}/${source}"
          COMMENT "Copying ${source} to build area")
    endforeach()
  endif()

  foreach( c_input ${c_inputs} )
    get_filename_component(basename "${c_input}" NAME_WE)
    add_custom_command(
      OUTPUT "${basename}${CMAKE_C_OUTPUT_EXTENSION}"
      COMMAND "${OCAMLFIND}" "ocamlc" "-c" "${c_input}" -ccopt ${c_flags}
      DEPENDS "${c_input}"
      COMMENT "Building OCaml stub object file ${basename}${CMAKE_C_OUTPUT_EXTENSION}"
      VERBATIM)
  endforeach()

  set(ocaml_params)
  foreach( ocaml_input ${ocaml_inputs} ${c_outputs})
    get_filename_component(filename "${ocaml_input}" NAME)
    list(APPEND ocaml_params "${filename}")
  endforeach()

  if( APPLE )
    set(ocaml_rpath "@executable_path/../../lib")
  elseif( UNIX )
    set(ocaml_rpath "\\$ORIGIN/../../lib")
  endif()
  list(APPEND ocaml_flags "-ldopt" "-Wl,-rpath,${ocaml_rpath}")

  add_custom_command(
    OUTPUT ${ocaml_outputs}
    COMMAND "${OCAMLFIND}" "ocamlmklib" "-o" "${name}" ${ocaml_flags} ${ocaml_params}
    DEPENDS ${ocaml_inputs} ${c_outputs}
    COMMENT "Building OCaml library ${name}"
    VERBATIM)

  add_custom_command(
    OUTPUT "${bin}/${name}.odoc"
    COMMAND "${OCAMLFIND}" "ocamldoc"
            "-I" "${bin}"
            "-I" "${LLVM_LIBRARY_DIR}/ocaml/"
            "-dump" "${bin}/${name}.odoc"
            ${ocaml_pkgs} ${ocaml_inputs}
    DEPENDS ${ocaml_inputs} ${ocaml_outputs}
    COMMENT "Building OCaml documentation for ${name}"
    VERBATIM)

  add_custom_target("ocaml_${name}" ALL DEPENDS ${ocaml_outputs} "${bin}/${name}.odoc")

  set_target_properties("ocaml_${name}" PROPERTIES
    OCAML_FLAGS "-I;${bin}")
  set_target_properties("ocaml_${name}" PROPERTIES
    OCAML_ODOC "${bin}/${name}.odoc")

  foreach( ocaml_dep ${ARG_OCAMLDEP} )
    add_dependencies("ocaml_${name}" "ocaml_${ocaml_dep}")
  endforeach()

  foreach( llvm_lib ${llvm_libs} )
    add_dependencies("ocaml_${name}" "${llvm_lib}")
  endforeach()

  set(install_files)
  set(install_shlibs)
  foreach( ocaml_output ${ocaml_outputs} )
    get_filename_component(ext "${ocaml_output}" EXT)

    if( NOT (ext STREQUAL ".cmo" OR
             ext STREQUAL CMAKE_C_OUTPUT_EXTENSION OR
             ext STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX) )
      list(APPEND install_files "${ocaml_output}")
    elseif( ext STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
      list(APPEND install_shlibs "${ocaml_output}")
    endif()
  endforeach()

  install(FILES ${install_files}
          DESTINATION lib/ocaml)
  install(FILES ${install_shlibs}
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                      GROUP_READ GROUP_EXECUTE
                      WORLD_READ WORLD_EXECUTE
          DESTINATION lib/ocaml)

  foreach( install_file ${install_files} ${install_shlibs} )
    get_filename_component(filename "${install_file}" NAME)
    add_custom_command(TARGET "ocaml_${name}" POST_BUILD
      COMMAND "${CMAKE_COMMAND}" "-E" "copy" "${install_file}"
                                             "${LLVM_LIBRARY_DIR}/ocaml/"
      COMMENT "Copying OCaml library component ${filename} to intermediate area"
      VERBATIM)
  endforeach()
endfunction()
