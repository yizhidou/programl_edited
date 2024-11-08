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


class UnsupportedCompiler(TypeError):
    """Exception raised if the requested compiler is not supported."""


class GraphCreationError(ValueError):
    """Exception raised if a graph creation op fails."""


class GraphTransformError(ValueError):
    """Exception raised if a graph transform op fails."""

# yzd added

class ResultsCreationError(ValueError):
    """Exception raised if a result creation op fails"""

class ValidationError(ValueError):
    """Exception raised if validation unpassed"""