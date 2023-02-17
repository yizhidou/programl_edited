# Copyright 2019-2020 the ProGraML authors.
#
# Contact Chris Cummins <chrisc.101@gmail.com>.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Graph serialization ops are used for storing or transferring Program
Graphs.
"""
import gzip
from pathlib import Path
from typing import Iterable, List, Optional, Union

import google.protobuf.message
import google.protobuf.text_format

from programl.exceptions import GraphCreationError, ResultsCreationError
from programl.proto import ProgramGraph, ProgramGraphList, ResultsEveryIteration, ResultsEveryIterationList


def save_graphs(
    path: Path, graphs: Iterable[ProgramGraph], compression: Optional[str] = "gz"
) -> None:
    """Save a sequence of program graphs to a file.

    :param path: The file to write.

    :param graphs: A sequence of Program Graphs.

    :param compression: Either :code:`gz` for GZip compression (the default), or
        :code:`None` for no compression. Compression increases the cost of
        serializing and deserializing but can greatly reduce the size of the
        serialized graphs.

    :raises TypeError: If an unsupported :code:`compression` is given.
    """
    with open(path, "wb") as f:
        f.write(to_bytes(graphs, compression=compression))


def load_graphs(
    path: Path, idx_list: Optional[List[int]] = None, compression: Optional[str] = "gz"
) -> List[ProgramGraph]:
    """Load program graphs from a file.

    :param path: The file to read from.

    :param idx_list: A zero-based list of graph indices to return. If not
        provided, all graphs are loaded.

    :param compression: Either :code:`gz` for GZip compression (the default), or
        :code:`None` for no compression. Compression increases the cost of
        serializing and deserializing but can greatly reduce the size of the
        serialized graphs.

    :return: A sequence of Program Graphs.

    :raises TypeError: If an unsupported :code:`compression` is given.

    :raise GraphCreationError: If deserialization fails.
    """
    with open(path, "rb") as f:
        return from_bytes(f.read(), idx_list=idx_list, compression=compression)


def to_bytes(
    graphs: Iterable[ProgramGraph], compression: Optional[str] = "gz"
) -> bytes:
    """Serialize a sequence of Program Graphs to a byte array.

    :param graphs: A sequence of Program Graphs.

    :param compression: Either :code:`gz` for GZip compression (the default), or
        :code:`None` for no compression. Compression increases the cost of
        serializing and deserializing but can greatly reduce the size of the
        serialized graphs.

    :return: The serialized program graphs.
    """
    compressors = {
        "gz": gzip.compress,
        None: lambda d: d,
    }
    if compression not in compressors:
        compressors = ", ".join(sorted(str(x) for x in compressors))
        raise TypeError(
            f"Invalid compression argument: {compression}. "
            f"Supported compressions: {compressors}"
        )
    compress = compressors[compression]

    return compress(ProgramGraphList(graph=list(graphs)).SerializeToString())


def from_bytes(
    data: bytes, idx_list: Optional[List[int]] = None, compression: Optional[str] = "gz"
) -> List[ProgramGraph]:
    """Deserialize Program Graphs from a byte array.

    :param data: The serialized Program Graphs.

    :param idx_list: A zero-based list of graph indices to return. If not
        provided, all graphs are returned.

    :param compression: Either :code:`gz` for GZip compression (the default), or
        :code:`None` for no compression. Compression increases the cost of
        serializing and deserializing but can greatly reduce the size of the
        serialized graphs.

    :return: A list of Program Graphs.

    :raise GraphCreationError: If deserialization fails.
    """
    decompressors = {
        "gz": gzip.decompress,
        None: lambda d: d,
    }
    if compression not in decompressors:
        decompressors = ", ".join(sorted(str(x) for x in decompressors))
        raise TypeError(
            f"Invalid compression argument: {compression}. "
            f"Supported compressions: {decompressors}"
        )
    decompress = decompressors[compression]

    graph_list = ProgramGraphList()
    try:
        graph_list.ParseFromString(decompress(data))
    except (gzip.BadGzipFile, google.protobuf.message.DecodeError) as e:
        raise GraphCreationError(str(e)) from e

    if idx_list:
        return [graph_list.graph[i] for i in idx_list]
    return list(graph_list.graph)


def to_string(graphs: Iterable[ProgramGraph]) -> str:
    """Serialize a sequence of Program Graphs to a human-readable string.

    The generated string has a JSON-like syntax that is designed for human
    readability. This is the least compact form of serialization.

    :param graphs: A sequence of Program Graphs.

    :return: The serialized program graphs.
    """
    return str(ProgramGraphList(graph=list(graphs)))


def from_string(
    string: str, idx_list: Optional[List[int]] = None
) -> List[ProgramGraph]:
    """Deserialize Program Graphs from a human-readable string.

    :param data: The serialized Program Graphs.

    :param idx_list: A zero-based list of graph indices to return. If not
        provided, all graphs are returned.

    :return: A list of Program Graphs.

    :raise GraphCreationError: If deserialization fails.
    """
    graph_list = ProgramGraphList()
    try:
        google.protobuf.text_format.Merge(string, graph_list)
    except google.protobuf.text_format.ParseError as e:
        raise GraphCreationError(str(e)) from e

    if idx_list:
        return [graph_list.graph[i] for i in idx_list]
    return list(graph_list.graph)

# yzd added

def dump_results_to_bytes(results: Iterable[ResultsEveryIteration],
                          compression: Optional[str] = "gz") -> bytes:
    """Serialize a sequence of YZD analysis results (ResultsEveryIteration) to a byte array.

    :param results: A sequence of ResultsEveryIteration.

    :param compression: Either :code:`gz` for GZip compression (the default), or
        :code:`None` for no compression. 

    :return: The serialized results.
    """
    compressors = {
        "gz": gzip.compress,
        None: lambda d: d,
    }
    if compression not in compressors:
        compressors = ", ".join(sorted(str(x) for x in compressors))
        raise TypeError(
            f"Invalid compression argument: {compression}. "
            f"Supported compressions: {compressors}"
        )
    compress = compressors[compression]

    return compress(ResultsEveryIterationList(list_of_results=list(results)).SerializeToString())

def load_results_from_bytes(data: bytes,
                            idx_list: Optional[List[int]] = None,
                            compression: Optional[str] = "gz") -> List[ProgramGraph]:
    """Deserialize ResultsEveryIteration(s) from a byte array.

    :param data: The serialized ResultsEveryIterationList.

    :param idx_list: A zero-based list of graph indices to return. If not
        provided, all graphs are returned.

    :param compression: Either :code:`gz` for GZip compression (the default), or
        :code:`None` for no compression. 

    :return: A list of ResultsEveryIteration(s).

    :raise ResultsCreationError: If deserialization fails.
    """
    decompressors = {
        "gz": gzip.decompress,
        None: lambda d: d,
    }
    if compression not in decompressors:
        decompressors = ", ".join(sorted(str(x) for x in decompressors))
        raise TypeError(
            f"Invalid compression argument: {compression}. "
            f"Supported compressions: {decompressors}"
        )
    decompress = decompressors[compression]

    results_list = ResultsEveryIterationList()
    try:
        results_list.ParseFromString(decompress(data))
    except (gzip.BadGzipFile, google.protobuf.message.DecodeError) as e:
        raise ResultsCreationError(str(e)) from e

    if idx_list:
        return [results_list.list_of_results[i] for i in idx_list]
    return list(results_list.list_of_results)

class ResultListSerializer:
    def serialize(self, results: Iterable[ResultsEveryIteration],
                 format: str,
                 compression: Optional[str] = "gz") -> bytes:
        _serializer = self._get_serializer(format=format)
        return _serializer(results, compression)

    def _get_serializer(self, format):
        if format == "byte" or format == "Byte" or format == "BYTE":
            return self._serialize_to_bytes
        pass

    def _serialize_to_bytes(self, results: Iterable[ResultsEveryIteration],
                            compression: Optional[str] = "gz"):
        """Serialize a sequence of ResultsEveryIteration(s) to a byte array.

        :results: A sequence of ResultsEveryIteration(s).

        :param compression: Either :code:`gz` for GZip compression (the default), or
            :code:`None` for no compression. Compression increases the cost of
            serializing and deserializing but can greatly reduce the size of the
            serialized graphs.

        :return: The serialized program graphs.
        """
        compressors = {
            "gz": gzip.compress,
            None: lambda d: d,
        }
        if compression not in compressors:
            compressors = ", ".join(sorted(str(x) for x in compressors))
            raise TypeError(
                f"Invalid compression argument: {compression}. "
                f"Supported compressions: {compressors}"
            )
        compress = compressors[compression]

        return compress(ResultsEveryIterationList(result=list(results)).SerializeToString())

    def _serialize_to_string(self, results: Iterable[ResultsEveryIteration]) -> str:
        """Serialize a sequence of ResultsEveryIteration(s) to a human-readable string.

        The generated string has a JSON-like syntax that is designed for human
        readability. This is the least compact form of serialization.

        :param results: A sequence of ResultsEveryIteration(s).

        :return: The serialized results.
        """
        return str(ResultsEveryIterationList(result=list(results)))

class ResultListLoader:
    def __init__(self) -> None:
        pass

