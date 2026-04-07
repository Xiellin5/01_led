function pid_tuner_gui()
    % 创建主窗口
    fig = figure('Name','PID参数调节界面','Position',[100,100,900,650],...
        'Resize','off','NumberTitle','off');
    
    % 初始化参数（非负初始值）
    kp_val = 1.0;
    ki_val = 0.1;
    kd_val = 0.05;
    setpoint_val = 1.0;  % 默认预期值
    
    % ========== 创建UI组件 ==========
    % 1. 绘图区域
    ax = axes('Parent',fig,'Position',[0.05,0.1,0.6,0.8]);
    hold(ax,'on');
    grid(ax,'on');
    xlabel(ax,'时间 (s)');
    ylabel(ax,'输出值');
    title(ax,'PID控制响应曲线');
    
    % 2. 预期值（设定值）调节区域
    uicontrol('Parent',fig,'Style','text','Position',[700,550,80,25],...
        'String','预期值:','FontSize',12);
    setpoint_slider = uicontrol('Parent',fig,'Style','slider','Position',[700,520,150,25],...
        'Min',0,'Max',10,'Value',setpoint_val,'SliderStep',[0.01,0.1]);
    setpoint_edit = uicontrol('Parent',fig,'Style','edit','Position',[860,520,60,25],...
        'String',num2str(setpoint_val),'FontSize',12);
    setpoint_error = uicontrol('Parent',fig,'Style','text','Position',[700,495,220,20],...
        'String','','ForegroundColor','red','FontSize',10);
    
    % 3. PID参数调节区域（右侧）
    % KP参数组
    uicontrol('Parent',fig,'Style','text','Position',[700,470,60,25],...
        'String','KP (比例):','FontSize',12);
    kp_slider = uicontrol('Parent',fig,'Style','slider','Position',[700,440,150,25],...
        'Min',0,'Max',10,'Value',kp_val,'SliderStep',[0.01,0.1]);
    kp_edit = uicontrol('Parent',fig,'Style','edit','Position',[860,440,60,25],...
        'String',num2str(kp_val),'FontSize',12);
    kp_error = uicontrol('Parent',fig,'Style','text','Position',[700,415,220,20],...
        'String','','ForegroundColor','red','FontSize',10);
    
    % KI参数组
    uicontrol('Parent',fig,'Style','text','Position',[700,395,60,25],...
        'String','KI (积分):','FontSize',12);
    ki_slider = uicontrol('Parent',fig,'Style','slider','Position',[700,365,150,25],...
        'Min',0,'Max',5,'Value',ki_val,'SliderStep',[0.01,0.1]);
    ki_edit = uicontrol('Parent',fig,'Style','edit','Position',[860,365,60,25],...
        'String',num2str(ki_val),'FontSize',12);
    ki_error = uicontrol('Parent',fig,'Style','text','Position',[700,340,220,20],...
        'String','','ForegroundColor','red','FontSize',10);
    
    % KD参数组
    uicontrol('Parent',fig,'Style','text','Position',[700,320,60,25],...
        'String','KD (微分):','FontSize',12);
    kd_slider = uicontrol('Parent',fig,'Style','slider','Position',[700,290,150,25],...
        'Min',0,'Max',2,'Value',kd_val,'SliderStep',[0.01,0.1]);
    kd_edit = uicontrol('Parent',fig,'Style','edit','Position',[860,290,60,25],...
        'String',num2str(kd_val),'FontSize',12);
    kd_error = uicontrol('Parent',fig,'Style','text','Position',[700,265,220,20],...
        'String','','ForegroundColor','red','FontSize',10);
    
    % 4. 刷新按钮
    refresh_btn = uicontrol('Parent',fig,'Style','pushbutton','Position',[700,210,220,40],...
        'String','更新PID曲线','FontSize',12,'Callback',@update_pid_plot);
    
    % ========== 回调函数定义 ==========
    % 滑块联动输入框
    set(setpoint_slider,'Callback',@(~,~) slider2edit(setpoint_slider,setpoint_edit));
    set(kp_slider,'Callback',@(~,~) slider2edit(kp_slider,kp_edit));
    set(ki_slider,'Callback',@(~,~) slider2edit(ki_slider,ki_edit));
    set(kd_slider,'Callback',@(~,~) slider2edit(kd_slider,kd_edit));
    
    % 初始化绘制默认曲线
    update_pid_plot();
    
    % ------------------------------
    % 滑块值同步到输入框
    function slider2edit(slider,edit_box)
        val = get(slider,'Value');
        set(edit_box,'String',sprintf('%.3f',val));
    end
    
    % ------------------------------
    % 检查输入合法性并更新曲线
    function update_pid_plot(~,~)
        % 1. 读取并检查所有参数
        is_valid = true;
        
        % 检查预期值
        [setpoint, sp_ok] = check_non_negative(setpoint_edit,'预期值',setpoint_error);
        % 检查KP
        [kp, kp_ok] = check_non_negative(kp_edit,'KP',kp_error);
        % 检查KI
        [ki, ki_ok] = check_non_negative(ki_edit,'KI',ki_error);
        % 检查KD
        [kd, kd_ok] = check_non_negative(kd_edit,'KD',kd_error);
        
        is_valid = sp_ok && kp_ok && ki_ok && kd_ok;
        
        % 2. 参数合法则绘制曲线
        if is_valid
            % 清除原有曲线
            cla(ax);
            hold(ax,'on');
            grid(ax,'on');
            xlabel(ax,'时间 (s)');
            ylabel(ax,'输出值');
            title(ax,sprintf('PID控制响应 (设定值=%.3f, KP=%.3f, KI=%.3f, KD=%.3f)',...
                setpoint,kp,ki,kd));
            
            % 模拟一阶惯性系统的PID控制
            Ts = 0.001;          % 采样时间
            t_total = 1;       % 总仿真时间
            t = 0:Ts:t_total;   % 时间向量
            y = zeros(size(t)); % 系统输出
            e = zeros(size(t)); % 误差
            u = zeros(size(t)); % 控制量
            integral = 0;       % 积分项
            last_e = 0;         % 上一时刻误差
            
            % 目标值（阶跃信号）
            setpoint_vec = setpoint * ones(size(t));
            
            % PID控制仿真
            for i = 2:length(t)
                e(i) = setpoint_vec(i) - y(i-1);          % 计算误差
                integral = integral + e(i)*Ts;            % 积分项
                derivative = (e(i) - last_e)/Ts;          % 微分项
                
                % PID输出（控制量）
                u(i) = kp*e(i) + ki*integral + kd*derivative;
                
                % 一阶惯性系统响应 (T=1, K=1)
                y(i) = y(i-1) + Ts*(u(i) - y(i-1));
                
                last_e = e(i);
            end
            
            % 绘制曲线
            plot(ax,t,setpoint_vec,'--r','LineWidth',1.5,'DisplayName','设定值');
            plot(ax,t,y,'b','LineWidth',2,'DisplayName','PID输出');
            legend(ax,'Location','best');
            
            % ========== 核心改进：自动适配Y轴范围 ==========
            % 计算Y轴范围，预留20%余量显示超调
            y_max = max([y,setpoint]);
            y_min = min(y);
            y_range = y_max - y_min;
            
            % 防止y_range为0（无变化）
            if y_range < 1e-6
                y_range = setpoint * 0.2; % 基于设定值的默认余量
            end
            
            % 设置Y轴范围，确保超调完全显示
            ylim(ax, [y_min - 0.1*y_range, y_max + 0.1*y_range]);
        end
    end
    
    % ------------------------------
    % 检查输入是否为非负数
    function [val, is_ok] = check_non_negative(edit_box, param_name, error_text)
        str = get(edit_box,'String');
        % 尝试转换为数值
        try
            val = str2double(str);
            if isnan(val)
                set(error_text,'String',sprintf('%s: 请输入有效数字',param_name));
                is_ok = false;
            elseif val < 0
                set(error_text,'String',sprintf('%s: 不能为负数 (当前=%.3f)',param_name,val));
                is_ok = false;
            else
                set(error_text,'String',''); % 清除错误提示
                is_ok = true;
            end
        catch
            set(error_text,'String',sprintf('%s: 输入格式错误',param_name));
            val = 0;
            is_ok = false;
        end
    end

end