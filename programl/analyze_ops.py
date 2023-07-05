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

def yzd_analyze_cmd_list(task_name: str,
                         max_iteration: int,
                         program_graph_sourcepath: str,
                         edge_list_savepath: Union[None, str] = None,
                         result_savepath: Union[None, str] = None,
                         if_sync: bool = False,
                         if_idx_reorganized: bool = True):
    cmd_list = [ANALYZE_BINARY, task_name, '--max_iteration', str(max_iteration), '--program_graph_sourcepath', program_graph_sourcepath]
    if edge_list_savepath:
        cmd_list.append('--edge_list_savepath')
        cmd_list.append(edge_list_savepath)
    if result_savepath:
        cmd_list.append('--result_savepath')
        cmd_list.append(result_savepath)
    cmd_list.append('--sync=' + ('true' if if_sync else 'false'))
    cmd_list.append('--idx_reorganized=' + ('true' if if_idx_reorganized else 'false'))
    return cmd_list


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
                program_graph_sourcepath: str,
                edge_list_savepath: Union[None, str] = None,
                result_savepath: Union[None, str] = None,
                if_sync: bool = False,
                if_idx_reorganized: bool = True,
                timeout: int = 300):
    process = subprocess.Popen(args=yzd_analyze_cmd_list(task_name, max_iteration, program_graph_sourcepath, edge_list_savepath, result_savepath, if_sync, if_idx_reorganized),
                               stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
    try:
        stdout, stderr = process.communicate(timeout=timeout)
    except subprocess.TimeoutExpired as e:
        raise TimeoutError(str(e)) from e

    return stdout, stderr


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
