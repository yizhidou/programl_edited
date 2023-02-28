import subprocess
import json
from programl.exceptions import ValidationError
from programl.util.py.runfiles_path import runfiles_path

VALIDATITION_BINARY = runfiles_path("programl/bin/validate")

def _dict_from_subprocess(process, stdout, stderr):
    if process.returncode:
        try:
            raise ValidationError(stderr.decode("utf-8"))
        except UnicodeDecodeError as e:
            raise ValidationError("Unknown error in validation") from e

    result_dict = dict()
    lines = stdout.decode("utf-8").strip().split('\n')
    assert (len(lines) == 7)
    for line in lines:
        k, v = line.split(' ')
        if k == "num_control_edge":
            num_control_edge = int(v)
        elif k == "num_data_edge":
            num_data_edge = int(v)
        elif k == "num_none_empty_gens_liveness":
            num_none_empty_gens_liveness = int(v)
        elif k == "num_none_empty_kills_liveness":
            num_none_empty_kills_liveness = int(v)
        else:
            result_dict[k.strip()] = int(v)
    result_dict['rate_non_empty_gens_liveness'] = num_none_empty_gens_liveness / float(result_dict['num_program_points'])
    result_dict['rate_non_empty_kills_liveness'] = num_none_empty_kills_liveness / float(result_dict['num_program_points'])
    result_dict['avg_control_degree'] = num_control_edge / float(result_dict['num_program_points'])
    result_dict['avg_data_degree'] = num_data_edge / float(result_dict['num_program_points'])
    return result_dict

def validate_one_file(src: str, timelimit: int, max_iteration: int = 500):
    process = subprocess.Popen(
            [VALIDATITION_BINARY, "--stdin_fmt=pb", str(max_iteration)],
            stdin=open(src),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    try:
        stdout, stderr = process.communicate(timeout=timelimit)
    except subprocess.TimeoutExpired as e:
        raise TimeoutError(str(e)) from e
    return _dict_from_subprocess(process, stdout, stderr)
    # subprocess.run(args=[VALIDATITION_BINARY, "--stdin_fmt=pb", str(max_iteration)], stdin=open(src), check=True)
