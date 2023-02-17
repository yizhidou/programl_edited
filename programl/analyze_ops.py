import subprocess, os
from typing import Union, Iterable, Optional
import google.protobuf.message

from programl.util.py.cc_system_includes import get_system_includes
from programl.util.py.executor import ExecutorLike, execute
from programl.util.py.runfiles_path import runfiles_path
from programl.exceptions import ResultsCreationError
from programl import ResultsEveryIteration

ANALYZE_BINARY = str(runfile_path("programl/bin/analyze"))

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
                targets: Union[None, str, Iterable[str]],
                timeout: int = 300,
                executor: Optional[ExecutorLike] = None,
                chunksize: Optional[int] = None,):
    def _run_one(src: str, target: Union[None, str]):
        if isinstance(target, str):
            out_stream = subprocess.PIPE
        else:
            out_stream = open(target, "w")
        process = subprocess.Popen(args=[ANALYZE_BINARY, task_name, max_iteration],
                                   stdin=subprocess.PIPE,
                                   stdout=out_stream,
                                   stderr=subprocess.STDOUT)
        # os.close(out_stream)
        try:
            stdout, stderr = process.communicate(src.encode("utf-8"), timeout=timeout)
        except subprocess.TimeoutExpired as e:
            raise TimeoutError(str(e)) from e
        return _yzd_result_from_subprocess(process, stdout, stderr)
        
    if isinstance(srcs, str):
        assert (isinstance(targets, str) or (targets is None)), "there is only one src, thus the target should be no more than one!"
        return _run_one(srcs, targets)
    return execute(_run_one, srcs, executor, chunksize)

def yzd_check(task_name: str, max_iteration: int):
    pass
