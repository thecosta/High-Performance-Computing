% Read and plot into gify original video. 
% by Bruno Costa Rendon


og_frames = cell(485);

for i = 0 : 484
    i
    if (i < 10)
        fname = strcat('discovery/enter/EnterExitCrossingPaths2cor000', int2str(i));
        fname = strcat(fname, '.jpg');
    else
        if(i < 100)
            fname = strcat('discovery/enter/EnterExitCrossingPaths2cor00', int2str(i));
            fname = strcat(fname, '.jpg');
        else
            fname = strcat('discovery/enter/EnterExitCrossingPaths2cor0', int2str(i));
            fname = strcat(fname, '.jpg');
        end
    end
    fname
    %og_frames{i+1} = zeros(288, 384);
    og_frames{i+1} = imread(fname);
end

%% 
h = figure;
%axis tight manual % this ensures that getframe() returns a consistent size
filename = 'enter_motion.gif';
for n = 1 : 480
    n
    % Draw plot for y = x.^n   
    imshow(og_frames{n})
    axis([0 384 0 288]);
    drawnow 
    % Capture the plot as an image 
    frame = getframe(h); 
    im = frame2im(frame); 
    [imind,cm] = rgb2ind(im,256); 
    % Write to the GIF File 
    if n == 1 
        imwrite(imind,cm,filename,'gif', 'Loopcount', inf, 'DelayTime',0.1); 
    else 
        imwrite(imind,cm,filename,'gif','WriteMode','append', 'DelayTime',0.1); 
    end 
  end




