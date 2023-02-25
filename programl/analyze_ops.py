import subprocess
import os
from typing import Union, Iterable, Optional
import google.protobuf.message

from programl.util.py.cc_system_includes import get_system_includes
from programl.util.py.executor import ExecutorLike, execute
from programl.util.py.runfiles_path import runfiles_path
from programl.exceptions import ResultsCreationError
from programl import ResultsEveryIteration

ANALYZE_BINARY = str(runfiles_path("programl/bin/analyze"))
CHECK_BINARY = str(runfiles_path("programl/bin/validate"))

def _yzd_result_from_subprocess(process, stdout, stderr):
    if process.returncode:
        try:
            raise ResultsCreationError(stderr.decode("utf-8"))
        except UnicodeDecodeError as e:
            raise ResultsCreationError("Unknown error in graph construction") from e

    try:
        results = ResultsEveryIteration()
        results.ParseFromString(stdout)
        return results
    except google.protobuf.message.DecodeError as e:
        raise ResultsCreationError(str(e)) from e

def yzd_analyze(task_name: str,
                max_iteration: int,
                srcs: Union[str, Iterable[str]],
                timeout: int = 300,
                executor: Optional[ExecutorLike] = None,
                chunksize: Optional[int] = None,):
    def _run_one(src: str):
        process = subprocess.Popen(args=[ANALYZE_BINARY, task_name, max_iteration],
                                   stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        # os.close(out_stream)
        try:
            stdout, stderr = process.communicate(src.encode("utf-8"), timeout=timeout)
        except subprocess.TimeoutExpired as e:
            raise TimeoutError(str(e)) from e
        return _yzd_result_from_subprocess(process, stdout, stderr)

    if isinstance(srcs, str):
        return _run_one(srcs)
    return execute(_run_one, srcs, executor, chunksize)

def yzd_check(task_name: str,
              max_iteration: int,
              srcs: Union[str, Iterable[str]],
              timeout: int = 300,
              executor: Optional[ExecutorLike] = None,
              chunksize: Optional[int] = None,):
    def _run_one(src: str):
        process = subprocess.Popen(args=[CHECK_BINARY, task_name, max_iteration],
                                   stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        # os.close(out_stream)
        try:
            stdout, stderr = process.communicate(src.encode("utf-8"), timeout=timeout)
        except subprocess.TimeoutExpired as e:
            raise TimeoutError(str(e)) from e
        return _yzd_result_from_subprocess(process, stdout, stderr)

    if isinstance(srcs, str):
        return _run_one(srcs)
    return execute(_run_one, srcs, executor, chunksize)
