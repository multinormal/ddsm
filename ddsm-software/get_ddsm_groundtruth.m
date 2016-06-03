function [groundtruth] = get_ddsm_groundtruth(overlay_filename)
%
% Get all groundtruth data about a mammogram from a .OVERLAY file.
%
% GT = GET_DDSM_GROUNDTRUTH(F) obtains the groundtruth about the mammogram
% described by the overlay file with filename F. Example usage is provided 
% below.
%
% GET_DDSM_GROUNDTRUTH returns a cell array; each cell is a description of one
% abnormality that has been annotated. Each abnormality has exactly one boundary 
% and a boundary can have zero or more cores. Each cell returned by 
% GET_DDSM_GROUNDTRUTH will be a struct with the following fields:
% 
% .lesion_type    --- a cell array of strings; see [1] for details. 
% .assessment     --- see [1] for details of this field. 
% .subtlety       --- see [1] for details of this field. 
% .pathology      --- see [1] for details of this field. 
% .annotations    --- a struct with the following fields:
%
%   .boundary     --- an anoymous function (see below).
%   .cores        --- a cell array of anonymous functions (see below).
%
% Each anonymous function provides a way to obtain a binary image that
% indicated which pixels belong to the annotation. All anonymous functions
% have the same function prototype (i.e., must be called in the same way);
% quite simply, each function handle takes a 2-D vector that specifies the
% number of rows and columns (in that order!) that the associated mammogram
% has. This information is availbale in the ICS file for the case, and
% on the DDSM website (e.g., see [2]).
%
% The benefit of returning anonymous functions rather than binary images 
% directly is that the anonymous functions can be stored using very little
% memory and the corresponding binary images can be computed very quickly,
% whereas the binary images themselves are quite large. You can simply save the 
% result of a call to GET_DDSM_GROUNDTRUTH as a .mat file, and then load it back 
% in later, therefore potentially saving lots of disk space.
%
%
% Example usage:
%
% % We need to have the file 'A_1580_1.LEFT_MLO.OVERLAY' in Matlab's current 
% working directory.
%
% >> groundtruth = get_ddsm_groundtruth('A_1580_1.LEFT_MLO.OVERLAY');
% >> groundtruth
%
% length(groundtruth) 
%
% ans =
% 
%      2
%
% % We see that groundtruth has two abnormalities. Let's look at the first:
%
% >> groundtruth{1}
% 
% ans = 
% 
%     lesion_type: {'MASS SHAPE IRREGULAR MARGINS ILL_DEFINED'}
%      assessment: 4
%        subtlety: 3
%       pathology: 'MALIGNANT'
%     annotations: [1x1 struct]
%
% % We see above the description that the radiologist recorded. Let's look at
% % the annotations that were made:
%
% >> groundtruth{1}.annotations
% 
% ans = 
% 
%     boundary: @(image_dims)make_annotation_image(image_dims,bc_text{i})
%        cores: {[1x1 function_handle]}
%
% % We see that we have a boundary (there is always one boundary for an 
% % abnormal image), and that its value is a function handle that takes
% % one parameter, image_dims. We also see that this abnormality has one
% % core annotation; we can verify this:
%
% >> length(groundtruth{1}.annotations.cores)
% 
% ans =
% 
%      1
% 
% % Let's now look at the boundary annotation; we need to know the dimensions
% % of the corresponding mammogram, so we look at the following webpage [2] and
% % see that for this mammogram, the ICS file says:
% % 
% %     "LEFT_MLO LINES 4606 PIXELS_PER_LINE 2221 ..."
% % 
% % We can now call the anonymous fuction that gets us the binary image that
% % indicates where the abnormality is:
%
% boundary_annotation = groundtruth{1}.annotations.boundary([4606 2221]);
%
% % And now we can display that image on our screen:
%
% >> imagesc(boundary_annotation)
%
% <Output not shown.>
%
% % We could also look at the core, this time directly rsther than saving it to
% % a variable:
%
% >> imagesc(groundtruth{1}.annotations.cores{1}([4606 2221]))
%
% <Output not shown.>
%
% % Let's now save this groundtruth to disk:
%
% >> save('some-groundtruth.mat', 'groundtruth')
%
% % We can now clear groundtruth from memory and then load it back again, 
% % simulating closing Matlab and then opening it again another day:
%
% >> clear all
% >> load('some-groundtruth.mat')
% >> imagesc(groundtruth{1}.annotations.cores{1}([4606 2221]))
%
% <Output not shown.>
%
% References:
%
% [1]: http://marathon.csee.usf.edu/Mammography/DDSM/case_description.html#OVERLAYFILE
% [2]: http://marathon.csee.usf.edu/Mammography/DDSM/thumbnails/cancers/cancer_10/case1580/A-1580-1.html

% Read the overlay file.
fhandle = fopen(overlay_filename, 'r');
all_lines = {};
while true
  all_lines{end+1} = fgetl(fhandle);
  if(all_lines{end} == -1)
    all_lines = all_lines(1:end-1);
    fclose(fhandle);
    break;
  end
end

% Find out how many abnormalities there are.
[x1, x2, x3, x4, x5, names] = ... % We ifnore the x variables.
 regexp(all_lines{1}, 'TOTAL_ABNORMALITIES (?<number>\d+)', 'names');
num_abnormalities = str2num(str2mat(names{1}));

% Get a call array, where the i-th cell is a description of the i-th abnormality.
abnormalities = {};
for(i = 1:num_abnormalities)
  abnormalities{i} = get_abnormality(i, all_lines);
end

groundtruth = abnormalities;


function [abnormality] = get_abnormality(abnormality_number, all_lines)
%
% From the cell array of file line strings all_lines, and an abnormality number,
% return a description of the specified abnormality.

abnormality = {};
found_abnormality = false;
for i = 1:length(all_lines)
  this_line = all_lines{i};
  % Find the abnormality we're looking for.
  if ~isempty(regexp(this_line, ['ABNORMALITY ' num2str(abnormality_number)]))
    found_abnormality = ~found_abnormality;
  end
  if found_abnormality

    abnormality{end+1} = this_line;
      
    % See if we've hit the next abnormality or the end of the file.
    if ~isempty(regexp(this_line, ['ABNORMALITY ' num2str(abnormality_number+1)]))
      abnormality = abnormality(1:end-1);
      break;  
    elseif i == length(all_lines)
      break;
    end


  end
end

% Now parse the abnormality.
abnormality = parse_abnormality(abnormality);


function [abnormality] = parse_abnormality(abnormality_text)
%
% Given a cell array containing lines of text for an abnormality, make a struct
% that encapsulates the information about the abnormality.

abnormality.lesion_type = {}; % Can have multiple lesion types.

for i = 1:length(abnormality_text)
  this_text = abnormality_text{i};
  
  [x1, x2, x3, x4, x5, vals] = regexp(this_text, ...
    'LESION_TYPE (?<lesion_type>.*)', 'names');
  if ~isempty(vals)
    abnormality.lesion_type{end+1} = vals{1}{1};
  end
  
  [x1, x2, x3, x4, x5, vals] = regexp(this_text, ...
    'ASSESSMENT (?<assessment>.*)', 'names');
  if ~isempty(vals)
    abnormality.assessment = str2num(vals{1}{1});
  end

  [x1, x2, x3, x4, x5, vals] = regexp(this_text, ...
    'SUBTLETY (?<subtlety>.*)', 'names');
  if ~isempty(vals)
    abnormality.subtlety = str2num(vals{1}{1});
  end

  [x1, x2, x3, x4, x5, vals] = regexp(this_text, ...
    'PATHOLOGY (?<pathology>.*)', 'names');
  if ~isempty(vals)
    abnormality.pathology = vals{1}{1};
  end

  % Get the number of outlines for this abnormality.
  [x1, x2, x3, x4, x5, vals] = regexp(this_text, ...
    'TOTAL_OUTLINES (?<total_outlines>.*)', 'names');
  if ~isempty(vals)
    num_outlines = str2num(vals{1}{1});
    total_outlines_line_number = i;
  end
  
end

% Now parse the boundary and any core specifications.
abnormality.annotations = parse_boundary_and_cores(abnormality_text(total_outlines_line_number+1:end));






function [annotations] = parse_boundary_and_cores(bc_text)
%
% From a cell array of strings that countain the lines from the "BOUNDARY" one
% up to but not including any subsequent abnormality (i.e., including "CORE"
% lines and their chain codes), return a struct with the members .boundary and 
% .cores, where .boundary is their chain code for the boundary annotation, and 
% .cores is a cell array of any core annotations, or an empty cell array 
% otherwise.

annotations.boundary = '';
annotations.cores = {};

for i=1:length(bc_text)
  % See if we are looking at a string that marks the next line as the boundary
  % or a chain code, or a chain code; remember what is next and extract the
  % chain code to the correct place when we come across it.
  if ~isempty(regexp(bc_text{i}, 'BOUNDARY'))
    code_is = 'boundary';
    continue;
  elseif ~isempty(regexp(bc_text{i}, 'CORE'))
    code_is = 'core';
    continue;
  end
  
  % We must be looking at a chain code. Make anonymous functions that, when 
  % called with a vector specifying the number of rows and columns in the 
  % corresponding mammogram, will return a binary image for the corresponding
  % annotation.
  switch code_is
    case 'boundary'
      annotations.boundary = ...
        @(image_dims) make_annotation_image(image_dims, bc_text{i});
    case 'core'
      annotations.cores{end+1} = ...
        @(image_dims) make_annotation_image(image_dims, bc_text{i});
  end
end


function [anno_image] = make_annotation_image(image_dims, chain_code)
%
% Given a vector of the number of rows and columns in the corresponding 
% mammogram and a chain code for an abnormality, return a binary image
% that indicates where the abnormality is.

% Remove the trailing '#' character.
if ~strcmp(chain_code(end), '#')
  error('Chain codes must end in a "#" character.');
end
chain_code = chain_code(1:end-1);

% Get the chain code as a vector.
chain_code = str2num(chain_code);

% Create a zero image of the right dimensions.
anno_image = zeros(image_dims);

% Now parse the chain code, filling in the edge pixels.
pos = [chain_code(2) chain_code(1)];
anno_image(pos(1), pos(2)) = 1;
for i = 3:length(chain_code)
  switch chain_code(i)
    case 0
      pos = pos + [-1 0];
    case 1
      pos = pos + [-1 1];
    case 2
      pos = pos + [0 1];
    case 3
      pos = pos + [1 1];
    case 4
      pos = pos + [1 0];
    case 5
      pos = pos + [1 -1];
    case 6
      pos = pos + [0 -1];
    case 7
      pos = pos + [-1 -1];
  end
  
  % Update this pixel.
  anno_image(pos(1), pos(2)) = 1;
end

% Fill the annotation.
anno_image = imfill(anno_image, 'holes');

if ~all(size(anno_image) == image_dims)
  error('There was a problem parsing the chain code.');
end






