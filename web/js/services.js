/**
 * Windows Services Manager Component
 */

class ServicesManager {
    static services = [];
    static searchQuery = '';
    static initialized = false;
    static refreshInterval = null;

    static initialize() {
        if (this.initialized) return;
        this.setupEventListeners();
        this.initialized = true;
    }

    static setupEventListeners() {
        const searchInput = document.getElementById('service-search');
        if (searchInput) {
            searchInput.addEventListener('input', (event) => {
                this.searchQuery = event.target.value.toLowerCase();
                this.render();
            });
        }
    }

    static async load() {
        this.initialize();

        try {
            const services = await api.getServices();
            this.services = Array.isArray(services) ? services : [];
            this.render();

            this.startAutoRefresh();
        } catch (error) {
            console.error('Failed to load services:', error);
            app.showNotification('Failed to load services', 'error');
        }
    }

    static startAutoRefresh() {
        if (this.refreshInterval) {
            clearInterval(this.refreshInterval);
        }

        this.refreshInterval = setInterval(async () => {
            try {
                const services = await api.getServices();
                this.services = Array.isArray(services) ? services : [];
                this.render();
            } catch (error) {
                // Silent fail
            }
        }, 10000);
    }

    static render() {
        const tableBody = document.getElementById('services-table-body');
        if (!tableBody) return;

        let services = [...this.services];

        if (this.searchQuery) {
            services = services.filter(service => {
                return (service.service_name || '').toLowerCase().includes(this.searchQuery) ||
                       (service.display_name || '').toLowerCase().includes(this.searchQuery);
            });
        }

        if (services.length === 0) {
            tableBody.innerHTML = `
                <tr>
                    <td colspan="5" class="empty-state">
                        <i class="fas fa-search"></i>
                        <p>${app.translate('no_services')}</p>
                    </td>
                </tr>
            `;
            return;
        }

        tableBody.innerHTML = services.map(service => {
            const statusClass = this.getStatusClass(service.status);
            const statusIcon = this.getStatusIcon(service.status);

            return `
                <tr>
                    <td><strong>${this.escapeHtml(service.service_name || 'Unknown')}</strong></td>
                    <td>${this.escapeHtml(service.display_name || 'Unknown')}</td>
                    <td>
                        <span class="status-badge ${statusClass}">
                            <i class="fas ${statusIcon}"></i> ${service.status || 'Unknown'}
                        </span>
                    </td>
                    <td>${service.startup_type || 'Unknown'}</td>
                    <td>
                        <div class="action-buttons">
                            ${service.status === 'Stopped' ? `
                                <button class="btn btn-sm btn-success" 
                                        onclick="ServicesManager.start('${this.escapeHtml(service.service_name)}')"
                                        title="Start service">
                                    <i class="fas fa-play"></i> Start
                                </button>
                            ` : `
                                <button class="btn btn-sm btn-danger" 
                                        onclick="ServicesManager.stop('${this.escapeHtml(service.service_name)}')"
                                        title="Stop service">
                                    <i class="fas fa-stop"></i> Stop
                                </button>
                            `}
                            <button class="btn btn-sm btn-info" 
                                    onclick="ServicesManager.restart('${this.escapeHtml(service.service_name)}')"
                                    title="Restart service">
                                <i class="fas fa-rotate"></i>
                            </button>
                        </div>
                    </td>
                </tr>
            `;
        }).join('');
    }

    static async start(serviceName) {
        try {
            await api.startService(serviceName);
            app.showNotification(`Service ${serviceName} started`, 'success');
            this.load();
        } catch (error) {
            console.error('Failed to start service:', error);
            app.showNotification(`Failed to start service ${serviceName}`, 'error');
        }
    }

    static async stop(serviceName) {
        const confirmed = await app.showConfirmation(
            `Are you sure you want to stop service "${serviceName}"?`,
            'Stop Service'
        );

        if (!confirmed) return;

        try {
            await api.stopService(serviceName);
            app.showNotification(`Service ${serviceName} stopped`, 'warning');
            this.load();
        } catch (error) {
            console.error('Failed to stop service:', error);
            app.showNotification(`Failed to stop service ${serviceName}`, 'error');
        }
    }

    static async restart(serviceName) {
        const confirmed = await app.showConfirmation(
            `Are you sure you want to restart service "${serviceName}"?`,
            'Restart Service'
        );

        if (!confirmed) return;

        try {
            await api.restartService(serviceName);
            app.showNotification(`Service ${serviceName} restarted`, 'success');
            this.load();
        } catch (error) {
            console.error('Failed to restart service:', error);
            app.showNotification(`Failed to restart service ${serviceName}`, 'error');
        }
    }

    static getStatusClass(status) {
        switch ((status || '').toLowerCase()) {
            case 'running': return 'status-running';
            case 'stopped': return 'status-stopped';
            case 'paused': return 'status-paused';
            default: return 'status-pending';
        }
    }

    static getStatusIcon(status) {
        switch ((status || '').toLowerCase()) {
            case 'running': return 'fa-check-circle';
            case 'stopped': return 'fa-times-circle';
            case 'paused': return 'fa-pause-circle';
            default: return 'fa-hourglass-half';
        }
    }

    static escapeHtml(str) {
        const div = document.createElement('div');
        div.textContent = str;
        return div.innerHTML;
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = { ServicesManager };
}