% Read and plot into gify filtered video. 
% by Bruno Costa Rendon

frames = cell(448);
for i = 1:448
    fname = strcat('discovery/filtered_text/', int2str(i));
    fname = strcat(fname, '.txt');
    frames{i} = dlmread(fname);
end

%%
x_coord = cell(448);
y_coord = cell(448);
for i = 1:448
    i
    count = 0;
    for j = 1 : 288
        for k = 1 : 384
            if (frames{i}(j, k) == 1)
                count = count + 1;
            end
        end
    end
    x_coord{i} = zeros(count);
    y_coord{i} = zeros(count);
end

for i = 1:448
    i
    count = 1;
    for j = 1 : 288
        for k = 1 : 384
            if (frames{i}(j, k) == 1)
                x_coord{i}(count) = k;
                y_coord{i}(count) = j;
                count = count + 1;
            end
        end
    end
end

%% 
h = figure;
%axis tight manual % this ensures that getframe() returns a consistent size
filename = 'motion.gif';
for n = 1 : 448
    n
    % Draw plot for y = x.^n   
    scatter(x_coord{n}(:), -y_coord{n}(:), 'b.')
    axis([0 384 -288 0]);
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


