# DirectX (ESIA_BACKEND_DIRECTX): the Direct3D 9, 10, 11 and 12 backends and what they share (common/). Each version is
# a backend of its own at run time (d3d9 ... d3d12): a host that creates its device picks one, an engine's renderer
# brings its own (the examples' --api directx starts the first that works: 11, 12, 10, 9). The advanced
# ESIA_BACKEND_D3D<version> options leave single versions out.
add_subdirectory(common)
foreach(_v 9 10 11 12)
    if(ESIA_BACKEND_D3D${_v})
        add_subdirectory(d3d${_v})
    endif()
endforeach()
