// Defacto Website - Enhanced JavaScript

document.addEventListener('DOMContentLoaded', function() {
    // ============================================
    // Code Tabs Functionality
    // ============================================
    const tabButtons = document.querySelectorAll('.tab-btn');
    const codeBlocks = document.querySelectorAll('.code-block');

    tabButtons.forEach(function(button) {
        button.addEventListener('click', function() {
            const targetTab = this.getAttribute('data-tab');

            // Remove active class from all buttons and blocks
            tabButtons.forEach(function(btn) {
                btn.classList.remove('active');
            });
            codeBlocks.forEach(function(block) {
                block.classList.remove('active');
            });

            // Add active class to clicked button and corresponding block
            this.classList.add('active');
            const targetBlock = document.getElementById(targetTab);
            if (targetBlock) {
                targetBlock.classList.add('active');
            }
        });
    });

    // ============================================
    // Install Tabs Functionality
    // ============================================
    const installTabButtons = document.querySelectorAll('.install-tab-btn');
    const installPanels = document.querySelectorAll('.install-panel');

    installTabButtons.forEach(function(button) {
        button.addEventListener('click', function() {
            const targetPlatform = this.getAttribute('data-platform');

            // Remove active class from all buttons and panels
            installTabButtons.forEach(function(btn) {
                btn.classList.remove('active');
            });
            installPanels.forEach(function(panel) {
                panel.classList.remove('active');
            });

            // Add active class to clicked button and corresponding panel
            this.classList.add('active');
            const targetPanel = document.getElementById(targetPlatform);
            if (targetPanel) {
                targetPanel.classList.add('active');
            }
        });
    });

    // ============================================
    // Smooth Scroll for Anchor Links
    // ============================================
    document.querySelectorAll('a[href^="#"]').forEach(function(anchor) {
        anchor.addEventListener('click', function(e) {
            const href = this.getAttribute('href');
            if (href !== '#' && href.length > 1) {
                e.preventDefault();
                const target = document.querySelector(href);
                if (target) {
                    target.scrollIntoView({
                        behavior: 'smooth',
                        block: 'start'
                    });
                }
            }
        });
    });

    // ============================================
    // Copy Code Functionality
    // ============================================
    const copyCodeButtons = document.querySelectorAll('.copy-code-btn');

    copyCodeButtons.forEach(function(button) {
        button.addEventListener('click', function() {
            const codeContainer = this.closest('.code-container');
            const activeCode = codeContainer.querySelector('.code-block.active');
            
            if (activeCode) {
                const code = activeCode.querySelector('code') || activeCode;
                const text = code.textContent;

                navigator.clipboard.writeText(text).then(function() {
                    const originalText = button.innerHTML;
                    button.innerHTML = '<span class="copy-text">Copied!</span>';
                    button.style.borderColor = 'var(--color-success)';
                    button.style.color = 'var(--color-success)';
                    
                    setTimeout(function() {
                        button.innerHTML = originalText;
                        button.style.borderColor = '';
                        button.style.color = '';
                    }, 2000);
                }).catch(function() {
                    const originalText = button.innerHTML;
                    button.innerHTML = '<span class="copy-text">Failed</span>';
                    button.style.borderColor = 'var(--color-error)';
                    button.style.color = 'var(--color-error)';
                    
                    setTimeout(function() {
                        button.innerHTML = originalText;
                        button.style.borderColor = '';
                        button.style.color = '';
                    }, 2000);
                });
            }
        });
    });

    // ============================================
    // Add Copy Button to All Code Blocks
    // ============================================
    const codeContainers = document.querySelectorAll('.doc-section pre, .install-step pre, .install-card pre, .command-example pre, .config-example pre, .package-creation pre, .setup-step pre, .code-side pre');

    codeContainers.forEach(function(container) {
        // Skip if already has a copy button
        if (container.querySelector('.copy-btn')) return;
        
        // Skip if inside a code-container (already handled)
        if (container.closest('.code-container')) return;

        const copyBtn = document.createElement('button');
        copyBtn.className = 'copy-btn';
        copyBtn.innerHTML = '<span class="copy-text">Copy</span>';
        copyBtn.style.cssText = 'position: absolute; right: 10px; top: 10px; background: var(--color-bg-tertiary); border: 1px solid var(--color-border); color: var(--color-text); padding: 5px 10px; border-radius: 4px; cursor: pointer; font-size: 0.8rem; display: flex; align-items: center; gap: 4px; transition: all 0.2s;';

        copyBtn.addEventListener('click', function() {
            const code = container.querySelector('code') || container;
            const text = code.textContent;

            navigator.clipboard.writeText(text).then(function() {
                copyBtn.innerHTML = '<span class="copy-text">Copied!</span>';
                copyBtn.style.borderColor = 'var(--color-success)';
                copyBtn.style.color = 'var(--color-success)';
                
                setTimeout(function() {
                    copyBtn.innerHTML = '<span class="copy-text">Copy</span>';
                    copyBtn.style.borderColor = '';
                    copyBtn.style.color = '';
                }, 2000);
            }).catch(function() {
                copyBtn.innerHTML = '<span class="copy-text">Failed</span>';
                copyBtn.style.borderColor = 'var(--color-error)';
                copyBtn.style.color = 'var(--color-error)';
                
                setTimeout(function() {
                    copyBtn.innerHTML = '<span class="copy-text">Copy</span>';
                    copyBtn.style.borderColor = '';
                    copyBtn.style.color = '';
                }, 2000);
            });
        });

        container.style.position = 'relative';
        container.appendChild(copyBtn);
    });

    // ============================================
    // Mobile Menu Toggle
    // ============================================
    const mobileMenuBtn = document.querySelector('.mobile-menu-btn');
    const navMenu = document.querySelector('.nav-menu');

    if (mobileMenuBtn && navMenu) {
        mobileMenuBtn.addEventListener('click', function() {
            navMenu.classList.toggle('active');
            this.classList.toggle('active');
        });

        // Close menu when clicking outside
        document.addEventListener('click', function(e) {
            if (!mobileMenuBtn.contains(e.target) && !navMenu.contains(e.target)) {
                navMenu.classList.remove('active');
                mobileMenuBtn.classList.remove('active');
            }
        });

        // Close menu when clicking on a link
        navMenu.querySelectorAll('a').forEach(function(link) {
            link.addEventListener('click', function() {
                navMenu.classList.remove('active');
                mobileMenuBtn.classList.remove('active');
            });
        });
    }

    // ============================================
    // Active Navigation Highlighting
    // ============================================
    const currentPage = window.location.pathname.split('/').pop() || 'index.html';
    const navLinks = document.querySelectorAll('.nav-link');

    navLinks.forEach(function(link) {
        const href = link.getAttribute('href');
        if (href === currentPage || (currentPage === '' && href === 'index.html')) {
            link.classList.add('active');
        }
    });

    // ============================================
    // Intersection Observer for Animations
    // ============================================
    const observerOptions = {
        threshold: 0.1,
        rootMargin: '0px 0px -50px 0px'
    };

    const observer = new IntersectionObserver(function(entries) {
        entries.forEach(function(entry) {
            if (entry.isIntersecting) {
                entry.target.style.opacity = '1';
                entry.target.style.transform = 'translateY(0)';
            }
        });
    }, observerOptions);

    // Observe feature cards, install cards, and other elements
    const animateElements = document.querySelectorAll('.feature-card, .install-card, .ecosystem-card, .community-card, .changelog-item');
    
    animateElements.forEach(function(el) {
        el.style.opacity = '0';
        el.style.transform = 'translateY(20px)';
        el.style.transition = 'opacity 0.5s ease, transform 0.5s ease';
        observer.observe(el);
    });

    // ============================================
    // Documentation Sidebar Scroll Spy
    // ============================================
    const docSections = document.querySelectorAll('.doc-section');
    const docNavLinks = document.querySelectorAll('.doc-nav-vertical a');

    if (docSections.length > 0 && docNavLinks.length > 0) {
        const docObserver = new IntersectionObserver(function(entries) {
            entries.forEach(function(entry) {
                if (entry.isIntersecting) {
                    const id = entry.target.getAttribute('id');
                    docNavLinks.forEach(function(link) {
                        link.classList.remove('active');
                        if (link.getAttribute('href') === '#' + id) {
                            link.classList.add('active');
                        }
                    });
                }
            });
        }, { threshold: 0.3 });

        docSections.forEach(function(section) {
            docObserver.observe(section);
        });
    }

    // ============================================
    // Syntax Highlighting (Basic)
    // ============================================
    const codeBlocks_all = document.querySelectorAll('pre code');
    
    codeBlocks_all.forEach(function(block) {
        const code = block.textContent;
        
        // Highlight keywords
        const keywords = ['var', 'const', 'struct', 'function', 'call', 'if', 'else', 'loop', 'stop', 'display', 'readkey', 'readchar', 'clear', 'reboot', 'color', 'free', 'static.pl>', '#Mainprogramm.start', '#Mainprogramm.end', '#NO_RUNTIME', '#SAFE', '#MOV', '#DRIVER', '<.de', '.>', '<drv.', '.dr>', '<<func', '>>'];
        
        let highlighted = code;
        
        // Highlight strings
        highlighted = highlighted.replace(/"([^"\\]|\\.)*"/g, function(match) {
            return '<span style="color: var(--color-secondary);">' + match + '</span>';
        });
        
        // Highlight comments
        highlighted = highlighted.replace(/\/\/.*/g, function(match) {
            return '<span style="color: var(--color-text-muted);">' + match + '</span>';
        });
        
        // Highlight numbers
        highlighted = highlighted.replace(/\b\d+\b/g, function(match) {
            return '<span style="color: var(--color-accent);">' + match + '</span>';
        });
        
        // Highlight keywords
        keywords.forEach(function(keyword) {
            const regex = new RegExp('\\b' + keyword.replace(/[.*+?^${}()|[\]\\]/g, '\\$&') + '\\b', 'g');
            highlighted = highlighted.replace(regex, function(match) {
                return '<span style="color: var(--color-primary);">' + match + '</span>';
            });
        });
        
        // Highlight types
        const types = ['i32', 'i64', 'u8', 'string', 'pointer'];
        types.forEach(function(type) {
            const regex = new RegExp('\\b' + type + '\\b', 'g');
            highlighted = highlighted.replace(regex, function(match) {
                return '<span style="color: var(--color-rust);">' + match + '</span>';
            });
        });
        
        block.innerHTML = highlighted;
    });

    // ============================================
    // Stats Counter Animation
    // ============================================
    const statValues = document.querySelectorAll('.stat-value');
    
    const statObserver = new IntersectionObserver(function(entries) {
        entries.forEach(function(entry) {
            if (entry.isIntersecting) {
                entry.target.style.opacity = '1';
                entry.target.style.transform = 'scale(1)';
            }
        });
    }, { threshold: 0.5 });
    
    statValues.forEach(function(stat) {
        stat.style.opacity = '0';
        stat.style.transform = 'scale(0.8)';
        stat.style.transition = 'opacity 0.5s ease, transform 0.5s ease';
        statObserver.observe(stat);
    });

    // ============================================
    // Header Scroll Effect
    // ============================================
    const header = document.querySelector('.header');
    let lastScroll = 0;

    window.addEventListener('scroll', function() {
        const currentScroll = window.pageYOffset;
        
        if (currentScroll > 100) {
            header.style.boxShadow = 'var(--shadow-md)';
            header.style.backgroundColor = 'rgba(22, 27, 34, 0.95)';
        } else {
            header.style.boxShadow = 'none';
            header.style.backgroundColor = 'rgba(22, 27, 34, 0.8)';
        }
        
        lastScroll = currentScroll;
    });

    // ============================================
    // External Links - Add Target Blank
    // ============================================
    const externalLinks = document.querySelectorAll('a[href^="http://"], a[href^="https://"]');
    
    externalLinks.forEach(function(link) {
        if (!link.closest('.nav-menu') && !link.hasAttribute('target')) {
            link.setAttribute('target', '_blank');
            link.setAttribute('rel', 'noopener noreferrer');
        }
    });

    // ============================================
    // Details/Summary Enhancement
    // ============================================
    const details = document.querySelectorAll('details');
    
    details.forEach(function(detail) {
        detail.addEventListener('click', function() {
            // Close other open details
            details.forEach(function(otherDetail) {
                if (otherDetail !== detail && otherDetail.open) {
                    otherDetail.open = false;
                }
            });
        });
    });

    // ============================================
    // Search Functionality (Future Enhancement)
    // ============================================
    // Placeholder for future search feature
    
    // ============================================
    // Console Welcome Message
    // ============================================
    console.log('%c Welcome to Defacto! ', 'background: #58a6ff; color: #0d1117; font-size: 20px; font-weight: bold; padding: 10px;');
    console.log('%c Low-level programming language for x86-32 ', 'color: #c9d1d9; font-size: 14px;');
    console.log('%c Learn more: https://github.com/vivooifo-droid/Defacto ', 'color: #8b949e; font-size: 12px;');

    // ============================================
    // Performance: Lazy Load Images (Future)
    // ============================================
    // Add lazy loading for images when they are added to the site
    
    // ============================================
    // Keyboard Navigation
    // ============================================
    document.addEventListener('keydown', function(e) {
        // Press 'Esc' to close mobile menu
        if (e.key === 'Escape') {
            if (navMenu && navMenu.classList.contains('active')) {
                navMenu.classList.remove('active');
                mobileMenuBtn.classList.remove('active');
            }
            
            // Close all details
            details.forEach(function(detail) {
                detail.open = false;
            });
        }
    });
});

// ============================================
// Utility Functions
// ============================================

// Debounce function for performance
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

// Throttle function for performance
function throttle(func, limit) {
    let inThrottle;
    return function(...args) {
        if (!inThrottle) {
            func.apply(this, args);
            inThrottle = true;
            setTimeout(() => inThrottle = false, limit);
        }
    };
}

// ============================================
// Export for potential module usage
// ============================================
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { debounce, throttle };
}
