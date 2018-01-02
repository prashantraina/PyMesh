import PyMesh
import numpy as np

def get_tet_orientations_raw(vertices, tets):
    """ Compute orientation of each tet.
    Arguments:
        vertice (``numpy.ndarray``): n by 3 matrix representing vertices.
        tets (``numpy.ndarray``): m by 4 matrix of vertex indices representing tets.

    Returns:
        orientations (``numpy.ndarray``): m list of ``float``s, where
            Positive number => tet is positively oriented.
                          0 => tet is degenerate.
            Negative number => tet is inverted.
    """
    return PyMesh.get_tet_orientations(vertices, tets);

def get_tet_orientations(mesh):
    """ A thin wrapper of ``get_tet_orientations_raw``.
    """
    return PyMesh.get_tet_orientations(mesh.vertices, mesh.voxels);
