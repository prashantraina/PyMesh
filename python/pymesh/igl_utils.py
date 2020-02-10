from PyMesh import unique_rows
from PyMesh import face_normals
from PyMesh import vertex_normals
from PyMesh import edge_normals
from PyMesh import orient_outward
from PyMesh import igl_embree_reorient_facets_raycast

def orient_faces(vertices, faces, outward=True):
    oriented_faces = orient_outward(vertices, faces)
    if not outward:
        reverse_indices = list(reversed(range(faces.shape[1])))
        oriented_faces = oriented_faces[:, reverse_indices]
    return oriented_faces

def reorient_facets_raycast(vertices, faces, outward=True):
    oriented_faces = igl_embree_reorient_facets_raycast(vertices, faces)
    if not outward:
        reverse_indices = list(reversed(range(faces.shape[1])))
        oriented_faces = oriented_faces[:, reverse_indices]
    return oriented_faces
