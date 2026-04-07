function drone_pid_controller()
    % 主函数：创建GUI界面
    fig = figure('Name','无人机PID高度控制调节界面','Position',[100,100,1000,600],'Resize','off');
    
    % ====================== 界面布局 ======================
    % 1. 绘图区域（左侧，占主要空间）
    ax = axes('Parent',fig,'Position',[0.05,0.1,0.6,0.8]);
    xlabel('时间 (s)'); ylabel('高度 (m)'); title('无人机高度响应曲线');
    grid on; hold on;
    
    % 修复点1：初始化时就创建绘图对象，避免空句柄问题
    h_plot = plot(ax, 0, 0, 'b-', 'LineWidth',1.5); % 实时高度曲线
    h_target = plot(ax, 0, 0, 'r--', 'LineWidth',1.5); % 目标高度线
    legend('实际高度','目标高度','Location','best');
    
    % 2. 参数输入区域（右侧）
    lbl_kp = uicontrol('Parent',fig,'Style','text','Position',[700,500,60,25],'String','Kp:');
    edit_kp = uicontrol('Parent',fig,'Style','edit','Position',[770,500,100,25],'String','5.0');
    lbl_ki = uicontrol('Parent',fig,'Style','text','Position',[700,460,60,25],'String','Ki:');
    edit_ki = uicontrol('Parent',fig,'Style','edit','Position',[770,460,100,25],'String','0.0');
    lbl_kd = uicontrol('Parent',fig,'Style','text','Position',[700,420,60,25],'String','Kd:');
    edit_kd = uicontrol('Parent',fig,'Style','edit','Position',[770,420,100,25],'String','2.0');
    lbl_target = uicontrol('Parent',fig,'Style','text','Position',[700,380,80,25],'String','目标高度(m):');
    edit_target = uicontrol('Parent',fig,'Style','edit','Position',[790,380,80,25],'String','0.6');
    
    % 2.2 参数校验提示标签（初始为空）
    lbl_check = uicontrol('Parent',fig,'Style','text','Position',[700,340,200,25],'String','','ForegroundColor','red');
    
    % 2.3 运行按钮
    btn_run = uicontrol('Parent',fig,'Style','pushbutton','Position',[700,280,180,40],'String','运行PID控制仿真',...
        'Callback',@(src,evt) run_pid_simulation(src,evt,ax,h_plot,h_target,edit_kp,edit_ki,edit_kd,edit_target,lbl_check));
    
    % ====================== 核心回调函数：PID仿真 ======================
    function run_pid_simulation(~,~,ax,h_plot,h_target,edit_kp,edit_ki,edit_kd,edit_target,lbl_check)
        % 1. 读取并校验输入参数
        [kp,ki,kd,target_h,valid] = check_parameters(edit_kp,edit_ki,edit_kd,edit_target,lbl_check);
        if ~valid, return; end % 参数不合法则终止
        
        % 2. 初始化仿真参数
        g = 9.81;          % 重力加速度 (m/s²)
        m = 1.0;           % 无人机质量 (kg)
        dt = 0.001;         % 仿真步长 (s)
        t_total = 300;       % 修复点2：缩短仿真时间，让曲线更清晰
        t = 0:dt:t_total;  % 时间数组
        h = zeros(size(t));% 高度数组（初始为0）
        v = zeros(size(t));% 速度数组（初始为0）
        err = zeros(size(t));% 误差数组
        err_sum = 0;       % 积分项累加
        err_prev = 0;      % 上一时刻误差（微分项用）
        
        % 3. 逐步仿真PID控制
        for i = 2:length(t)
            % 3.1 计算当前误差
            err(i) = target_h - h(i-1);
            
            % 3.2 PID控制律计算升力（控制输入）
            err_sum = err_sum + err(i)*dt;          % 积分项
            err_diff = (err(i) - err_prev)/dt;      % 微分项
            lift_force = kp*err(i) + ki*err_sum + kd*err_diff; % PID输出升力
            lift_force = max(lift_force, 0); % 升力非负（物理约束）
            
            % 3.3 动力学方程：F=ma → 加速度 = (升力 - 重力)/质量
            a = (lift_force - m*g)/m;
            
            % 3.4 积分更新速度和高度（欧拉法）
            v(i) = v(i-1) + a*dt;
            h(i) = h(i-1) + v(i)*dt;
            h(i) = max(h(i), 0); % 高度不能低于地面
            
            % 3.5 更新误差状态
            err_prev = err(i);
        end
        
        % 4. 更新绘图
        set(h_plot,'XData',t,'YData',h);
        set(h_target,'XData',t,'YData',target_h*ones(size(t)));
        
        % 修复点3：自动调整坐标轴范围，让曲线完整显示
        xlim(ax, [t(1), t(end)]);
        ylim(ax, [0, max(h)*1.2]);
        drawnow;
    end
    
    % ====================== 辅助函数：参数校验 ======================
    function [kp,ki,kd,target_h,valid] = check_parameters(edit_kp,edit_ki,edit_kd,edit_target,lbl_check)
        valid = true;
        kp = 0; ki = 0; kd = 0; target_h = 0;
        err_msg = '';
        
        % 读取输入并转换为数值
        try
            kp = str2double(get(edit_kp,'String'));
            ki = str2double(get(edit_ki,'String'));
            kd = str2double(get(edit_kd,'String'));
            target_h = str2double(get(edit_target,'String'));
        catch
            err_msg = '输入不是有效数字！';
            valid = false;
        end
        
        % 检查是否为非负数
        if valid
            if isnan(kp) || kp < 0, err_msg = 'Kp必须为非负数！'; valid = false;
            elseif isnan(ki) || ki < 0, err_msg = 'Ki必须为非负数！'; valid = false;
            elseif isnan(kd) || kd < 0, err_msg = 'Kd必须为非负数！'; valid = false;
            elseif isnan(target_h) || target_h < 0, err_msg = '目标高度必须为非负数！'; valid = false;
            end
        end
        
        % 更新提示标签
        set(lbl_check,'String',err_msg);
        if valid, set(lbl_check,'ForegroundColor','green','String','参数校验通过！'); end
    end
end